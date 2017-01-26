/** @file parseError.cpp
 * @brief Implementation file for the tmTraceFile.parseERROR() function.
 */

#include "tmtracefile.h"



/** @brief Parses an "ERROR" line.
 *
 * Parses a line from the trace file. The line is expected
 * to be an ERROR #cursor line.
 *
 * Returns true if all ok. False otherwise.
 */
bool tmTraceFile::parseERROR(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parseERROR(): Entry." << endl;
    }

    // ERROR #275452960:err=31013 tim=1075688943194
    regex reg("ERROR\\s(#\\d+):err=(\\d+).*");
    smatch match;

    // Extract the cursorID and error code.
    if (!regex_match(thisLine, match, reg)) {
        stringstream s;
        s << "parseERROR(): Cannot match regex against ERROR at line: "
          <<  mLineNumber << "." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parseERROR(): Exit." << endl;
        }

        return false;
    }

    string cursorID = match[1];
    unsigned errorCode = stoul(match[2], NULL, 10);

    // This could be an error in recursive SQL, but check if we have
    // a cursor for the cursorID which would indicate user level SQL.
    map<string, tmCursor *>::iterator i = findCursor(cursorID);

    // If we found it, it must be depth=0.
    // Otherwise, quietly ignore it, it's recursive.
    if (i != mCursors.end()) {
        // This is temporary *****************************
        *mOfs << setw(MAXLINENUMBER) << mLineNumber << ' '
              << setw(MAXLINENUMBER) << ' ' << ' '
              << setw(MAXLINENUMBER) << ' ' << ' '
//              << setw(MAXCURSORWIDTH) << ' ' << ' '
              << " ERROR: ORA-" << errorCode << endl;
        // This is temporary *****************************
    }

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parseERROR(): Exit." << endl;
    }

    return true;
}




