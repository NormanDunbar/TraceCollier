/** @file parseExecc.cpp
 * @brief Implementation file for the tmTraceFile.parseEXEC() function.
 */

#include "tmtracefile.h"


/** @brief Parses a "EXEC" line.
 *
 * Parses a line from the trace file. The line is expected
 * to be the EXEC #cursor line.
 *
 * The tmCursor associated with this PARSE is found, and its bind values
 * are extracted and merged into the SQL statement ready for output to
 * the report file.
 *
 * Returns true if all ok. False otherwise.
 */
bool tmTraceFile::parseEXEC(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parseEXEC(): Entry." << endl;
    }

    // EXEC #5923197424:...dep=0...
    regex reg("EXEC\\s(#\\d+).*?dep=(\\d+).*");
    smatch match;

    // Extract the cursorID and depth.
    if (!regex_match(thisLine, match, reg)) {
        cerr << "parseEXEC(): Cannot match regex against EXEC at line: "
             <<  mLineNumber << "." << endl;

        if (mOptions->verbose()) {
            *mDbg << "parseEXEC(): Cannot match regex against EXEC at line: "
                  <<  mLineNumber << "." << endl
                  << "parseEXEC(): Exit." << endl;
        }

        return false;
    }

    string cursorID = match[1];
    unsigned depth = stoul(match[2], NULL, 10);

    if (depth) {
        // Ignore this one.
        if (mOptions->verbose()) {
            *mDbg << "parseEXEC(): Ignoring EXEC with dep=" << depth << '.' << endl
                  << "parseEXEC(): Exit." << endl;
        }

        return true;
    }

    // Find the cursor for this exec.
    map<string, tmCursor *>::iterator i = findCursor(cursorID);
    if (i == mCursors.end()) {
        cerr << "parseEXEC(): Cursor " << cursorID << " not found." << endl;

        if (mOptions->verbose()) {
            *mDbg << "parseEXEC(): Cursor " << cursorID << " not found." << endl
                  << "parseEXEC(): Exit." << endl;
        }

        return false;
    }

    // We have a valid cursor. Extract the SQL.
    tmCursor *thisCursor = i->second;

    // This is temporary *****************************
    mOfs->width(MAXLINENUMBER);
    *mOfs << mLineNumber << ' '
          << "EXEC " << cursorID << thisCursor->sqlText() << endl;
    // This is temporary *****************************

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parseEXEC(): Exit." << endl;
    }

    return true;
}

