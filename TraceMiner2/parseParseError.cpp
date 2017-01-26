/** @file parseParseError.cpp
 * @brief Implementation file for the tmTraceFile.parsePARSEERROR() function.
 */

#include "tmtracefile.h"


/** @brief Parses a "PARSE ERROR" line.
 *
 * Parses a line from the trace file. The line is expected
 * to be the PARSE ERROR #cursor line.
 *
 * Returns true if all ok. False otherwise.
 */
bool tmTraceFile::parsePARSEERROR(const string &thisLine) {

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parsePARSEERROR(): Exit." << endl;
    }

    return true;
}


