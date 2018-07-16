/*
 * MIT License
 *
 * Copyright (c) 2017 Norman Dunbar
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/** @file parseParsing.cpp
 * @brief Implementation file for the tmTraceFile.parsePARSING() function.
 */

#include "tmtracefile.h"
#include "gnu.h"

#ifndef USE_REGEX
    #include "utilities.h"
#endif


/** @brief Parses a "PARSING IN CURSOR" line.
 *
 * @param thisLine &std::string. A reference to a single line from the trace file. This
 *        will always be the "PARSING IN CURSOR" line.
 * @return bool.
 *
 * Parses a line form the trace file. The line is expected
 * to be the PARSING IN CURSOR line. We are only interested in
 * cursors at depth = 0.
 *
 * Creates a tmCursor object and appends it to the map<string, tmCursor *>
 * that we use to hold these things, if the depth is zero.
 *
 * Returns true if all ok. False otherwise.
 */
bool tmTraceFile::parsePARSING(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parsePARSING(" << mLineNumber << "): Entry." << endl;
    }

    // PARSING IN CURSOR #4572676384 len=229 dep=1 ...

    bool matchOk = true;
    string cursorID = "";
    unsigned sqlLength = 0;
    //unsigned depth = 0;       // Removed for Issue 10.
    unsigned commandType = 0;

    // The SQL starts on the following line, not this one!
    unsigned sqlLine = mLineNumber + 1;

#ifdef USE_REGEX
    regex reg("PARSING IN CURSOR\\s(#\\d+)\\slen=(\\d+)\\sdep=(\\d+).*?oct=(\\d+).*");
    smatch match;

    // Extract the cursorID, the length, recursion depth and command type.
    if (regex_match(thisLine, match, reg)) {
        cursorID = match[1];
        sqlLength = stoul(match[2], NULL, 10);
        //depth = stoul(match[3], NULL, 10);        // Removed for Issue 10.
        commandType = stoul(match[4], NULL, 10);
    } else {
        matchOk = false;
    }
#else
    cursorID = getCursor(thisLine, &matchOk);
    if (matchOk) {
        sqlLength = getDigits(thisLine, "len=", &matchOk);
        if (matchOk) {
            //depth = getDigits(thisLine, "dep=", &matchOk);  // Removed - Issue 10
            //if (matchOk) {
                commandType = getDigits(thisLine, "oct=", &matchOk);
            }
        }
    }
#endif // USE_REGEX

    // Did it work?
    if (!matchOk) {
        stringstream s;
        s << "parsePARSING(): Cannot match against PARSING IN CURSOR at line: "
          <<  mLineNumber << "." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str() << endl
                  << "parsePARSING(): Exit." << endl;
        }

        return false;
    }

    // We only care about SQL at the defined depth, default = 0.
    // Removed for Issue #10, we need to keep all cursors.
    /*
    if (depth > mOptions->depth()) {
        // Ignore this one.
        if (mOptions->verbose()) {
            *mDbg << "parsePARSING(): Ignoring PARSING IN CURSOR with dep=" << depth << '.' << endl
                  << "parsePARSING(): Exit." << endl;
        }

        return true;
    }
    */

    tmCursor *thisCursor = new tmCursor(cursorID, sqlLength, sqlLine);
    if (!thisCursor) {
        stringstream s;
        s << "parsePARSING(): Cannot allocate a new tmCursor." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parsePARSING(): Exit." << endl;
        }

        return false;
    }

    // Set the command type for later use.
    thisCursor->setCommandType(commandType);

    // Tell the world, unless we are running quietly.
    if (!mOptions->quiet()) {
        cout << "Cursor: " << thisCursor->cursorId()
             << " created at line: " << thisCursor->sqlLineNumber() << endl;
    }

    // Extract the SQL Text into a stream. This handles end of line for us.
    string aLine;
    stringstream ss;
    string lineBreak = "<br>";

    if (!mOptions->html()) {
        lineBreak = "";
    }

    while (readTraceLine(&aLine)) {
        if (aLine.substr(0, 11) == "END OF STMT") {
            break;
        }

        // Append this new SQL line to the existing lines.
        if (!ss.str().empty()) {
            // Append a system dependent newline to the current text.
            // If we are in HTML mode, add a "<BR>" too.
            ss << lineBreak << endl;
        }
        // And append this new line of text.
        ss << aLine;
    }

    // An iterator for the insert into the map. AKA where are we?
    pair<map<string, tmCursor *>::iterator, bool> exists;

    // Stash this new cursor. If the cursor exists, update it. Maps are weird!
    // CusrorIDs are like Highlanders. There can be only one! ;)
    exists = mCursors.insert(pair<string, tmCursor *>(cursorID, thisCursor));

    // If the bool is false, the insert failed - already there.
    // So we simply update.
    //
    // Exists is a pair<mCursors::iterator, bool>. It indexes the map at the
    // position of the inserted or already existing cursor data.
    // Exists.second is the bool. True=inserted, False=already exists.
    // Exists.first is an iterator (pseudo pointer) to a pair <string, tmCursor *>.
    // Exists.first->first is the string, aka the cursorID.
    // Exists.first->second is the tmCursor pointer.
    //
    // Phew!

    // So, after all that, did we insert or find our cursor?
    if (!exists.second) {
        // Update existing cursor details. Only the
        // SQL details will have changed. At the moment.
        // And we have not yet parsed this SQL text.
        exists.first->second->setSQLLineNumber(sqlLine);
        exists.first->second->setSQLLength(sqlLength);
        exists.first->second->setSQLParseLine(0);
        delete thisCursor;
    }

    // Then set the SQL Text, regardless.
    exists.first->second->setSQLText(ss.str());

    // Verbose?
    if (mOptions->verbose()) {
        *mDbg << endl << "parsePARSING(): "
              << (exists.second ? "Creating" : "ReCreating") << " Cursor: "
              << exists.first->first << endl
              << *(exists.first->second)
              << "parsePARSING(): Exit." << endl;
    }

    return true;

}

