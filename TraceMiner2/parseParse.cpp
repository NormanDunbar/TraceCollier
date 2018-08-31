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

/** @file parseParse.cpp
 * @brief Implementation file for the tmTraceFile.parsePARSE() function.
 */

#include "tmtracefile.h"
#include "gnu.h"

#ifndef USE_REGEX
    #include "utilities.h"
#endif

/** @brief Parses a "PARSE" line.
 *
 * @param thisLine const string&. The line of text with "PARSE" in.
 * @return bool. Returns true if all ok. False otherwise.
 *
 * Parses a line from the trace file. The line is expected
 * to be the PARSE \#cursor line.
 *
 * The tmCursor associated with this PARSE is found, and updated to the new
 * source file line number. Only the most recent PARSE is stored for each tmCursor.
 */
bool tmTraceFile::parsePARSE(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parsePARSE(" << mLineNumber << "): Entry." << endl;
    }

    // PARSE #5924310096:c=0,e=28,p=0,cr=0,cu=0,mis=0,r=0,dep=0,og=4,plh=1388734953,tim=526735705337
    string cursorID = "";
    // unsigned depth = 0;      // Removed for Issue #10. See below.
    bool matchOk = true;

#ifdef USE_REGEX
    regex reg("PARSE\\s(#\\d+).*?dep=(\\d+).*");
    smatch match;

    // Extract the cursorID, the length and the depth.
    if (regex_match(thisLine, match, reg)) {
        cursorID = match[1];
        //depth = stoul(match[2], NULL, 10);    // Removed for Issue 10.
    } else {
        matchOk = false;
    }
#else
    cursorID = getCursor(thisLine, &matchOk);
    // Removed for Issue 10.
    //if (matchOk) {
        //depth = getDigits(thisLine, "dep=", &matchOk);
    //}
#endif  // USE_REGEX

    // Did it all work?
    if (!matchOk) {
        stringstream s;
        s << "parsePARSE(): Cannot match against PARSE at line: "
          <<  mLineNumber << "." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parsePARSE(" << mLineNumber << "): Exit." << endl;
        }

        return false;
    }

    // We only care about user level SQL, so only depth <= depth().
    // # Issue #10 - we need ALL parsed cursors as they can be cached on
    // close and then exec'd again at a higher depth. Sigh.
    /*
    if (depth > mOptions->depth()) {
        // Ignore this one.
        if (mOptions->verbose()) {
            *mDbg << "parsePARSE(): Ignoring PARSE with dep=" << depth << '.' << endl
                  << "parsePARSE(): Exit." << endl;
        }

        return true;
    }
    */


    // Find the existing cursor.
    map<string, tmCursor *>::iterator i = findCursor(cursorID);

    // Found?
    if (i != mCursors.end()) {
        // Yes. Thankfully! Update the PARSE line number and closed flag.
        // i is an iterator to a pair<string, tmCursor *>
        // So i->first is the string.
        // And i->second is the tmCursor pointer.
        i->second->setSQLParseLine(mLineNumber);
        i->second->setClosed(false);
    } else {
        // Not found. Oh dear!
        stringstream s;
        s << "parsePARSE(" << mLineNumber << "): Found PARSE for cursor " << cursorID
          << " but not found in existing cursor list." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parsePARSE(" << mLineNumber << "): Exit." << endl;
        }

        return false;
    }

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parsePARSE(" << mLineNumber << "): Exit." << endl;
    }

    return true;
}


