/** @file parseExec.cpp
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

    // EXEC #5924310096:c=0,e=31,p=0,cr=0,cu=0,mis=0,r=0,dep=0,og=4,plh=1388734953,tim=526735705392
    regex reg("EXEC\\s(#\\d+).*?dep=(\\d+).*");
    smatch match;

    // Extract the cursorID and depth.
    if (!regex_match(thisLine, match, reg)) {
        stringstream s;
        s << "parseEXEC(): Cannot match regex against EXEC at line: "
          <<  mLineNumber << "." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str() << endl
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
        stringstream s;
        s << "parseEXEC(): Cursor " << cursorID << " not found." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str() << endl
                  << "parseEXEC(): Exit." << endl;
        }

        return false;
    }

    // We have a valid cursor. Extract the SQL.
    tmCursor *thisCursor = i->second;

    // This is temporary *****************************
    //mOfs->width(MAXLINENUMBER);
    *mOfs << setw(MAXLINENUMBER) << mLineNumber << ' '
          << setw(MAXLINENUMBER) << thisCursor->sqlParseLine() << ' '
          << setw(MAXLINENUMBER) << thisCursor->sqlLineNumber() << ' '
//          << setw(MAXCURSORWIDTH) << cursorID << ' '
          << thisCursor->sqlText() << ' '
          << endl;
    // This is temporary *****************************

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parseEXEC(): Exit." << endl;
    }

    return true;
}

