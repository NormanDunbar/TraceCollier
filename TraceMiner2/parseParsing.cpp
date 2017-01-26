/** @file parseParsing.cpp
 * @brief Implementation file for the tmTraceFile.parsePARSING() function.
 */

#include "tmtracefile.h"

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
        *mDbg << "parsePARSING(): Entry." << endl;
    }

    // PARSING IN CURSOR #4572676384 len=229 dep=1 ...
    regex reg("PARSING IN CURSOR\\s(#\\d+)\\slen=(\\d+)\\sdep=(\\d+).*");
    smatch match;

    stringstream ss;

    // Extract the cursorID, the length and the depth.
    if (!regex_match(thisLine, match, reg)) {
        cerr << "parsePARSING(): Cannot match regex against PARSING IN CURSOR at line: "
             <<  mLineNumber << "." << endl;
        return false;
    }

    // Found it!
    // Extract the goodies! We definitely have all three matches.
    // Match[0] = "#12345678 len=123 dep=0"
    // Match[1] = "#12345678"
    // Match[2] = "123"
    // Match[3] = "0"

    // No validation necessary, they are digits etc, as required.

    // The SQL starts on the following line, not this one!
    unsigned sqlLine = mLineNumber + 1;
    string cursorID = match[1];
    unsigned sqlLength = stoul(match[2], NULL, 10);
    unsigned depth = stoul(match[3], NULL, 10);

    // We only care about user level SQL so depth 0 only gets saved away.
    // This does mean that SQL executed in a PL/SQL call is ignored though.
    // So far, that's what my customer(s) want. (Hello Rich.)
    if (depth) {
        // Ignore this one.
        if (mOptions->verbose()) {
            *mDbg << "parsePARSING(): Ignoring PARSING IN CURSOR with dep=" << depth << '.' << endl
                  << "parsePARSING(): Exit." << endl;
        }

        return true;
    }

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

    // Extract the SQL Text into a stream. This handles end of line for us.

    string aLine;

    while (readTraceLine(&aLine)) {
        if (aLine.substr(0, 11) == "END OF STMT") {
            break;
        }

        // Append this new SQL line to the existing lines.
        if (!ss.str().empty()) {
            // Append a system dependent newline to the current text.
            ss << endl;
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
        *mDbg << endl << "parsePARSING(): Creating Cursor: "
              << exists.first->first << endl
              << *(exists.first->second);
    }

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parsePARSING(): Exit." << endl;
    }

    return true;

}

