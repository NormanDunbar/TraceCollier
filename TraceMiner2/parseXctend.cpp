/** @file parseXctend.cpp
 * @brief Implementation file for the tmTraceFile.parseXCTEND() function.
 */

#include "tmtracefile.h"



/** @brief Parses a "XCTEND" line.
 *
 * Parses a line from the trace file. The line is expected
 * to be the XCTEND line indicating COMMIT or ROLLBACK.
 * It should be noted that this line has no cursor ID.
 *
 * Returns true if all ok. False otherwise.
 */
bool tmTraceFile::parseXCTEND(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parseXCTEND(" << mLineNumber << "): Entry." << endl;
    }

    // XCTEND rlbk=0, rd_only=0, tim=524545341395
    regex reg("XCTEND\\srlbk=(\\d+).*?rd_only=(\\d+).*");
    smatch match;

    // Extract the rollback and read only flags.
    if (!regex_match(thisLine, match, reg)) {
        stringstream s;
        s << "parseXCTEND(): Cannot match regex against XCTEND at line: "
          <<  mLineNumber << "." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parseXCTEND(): Exit." << endl;
        }

        return false;
    }

    unsigned rollBack = stoul(match[1], NULL, 10);
    unsigned readOnly = stoul(match[2], NULL, 10);

    // This is temporary *****************************
    *mOfs << setw(MAXLINENUMBER) << mLineNumber << ' '
          << setw(MAXLINENUMBER) << ' ' << ' '
          << setw(MAXLINENUMBER) << ' ' << ' '
//          << setw(MAXCURSORWIDTH) << ' ' << ' '
          << (rollBack ? "ROLLBACK " : "COMMIT ")
          << (readOnly ? "(Read Only)" : "(Read Write)") << endl;
    // This is temporary *****************************

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parseXCTEND(): Exit." << endl;
    }

    return true;
}

