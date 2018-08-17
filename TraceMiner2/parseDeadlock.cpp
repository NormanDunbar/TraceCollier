/*
 * MIT License
 *
 * Copyright (c) 2017-2018 Norman Dunbar
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

/** @file parseDeadlock.cpp
 * @brief Implementation file for the tmTraceFile.parseDEADLOCK() function.
 */

#include "tmtracefile.h"
#include "gnu.h"

#ifndef USE_REGEX
    #include "utilities.h"
#endif


/** @brief Parses a "DEADLOCK DETECTED" line.
 *
 * Parses a line from the trace file. The line is expected
 * to be a DEADLOCK DETECTED line.
 */
void tmTraceFile::parseDEADLOCK() {

    if (mOptions->verbose()) {
        *mDbg << "parseDEADLOCK(" << mLineNumber << "): Entry." << endl;
    }

    // DEADLOCK DETECTED ( ORA-00060 )

    // Stuff for the report.
    stringstream deadlockData;
    string deadlockLine;
    unsigned currentLineNumber = mLineNumber;

    // Scan the trace file for the "Deadlock graph:" line.
    while (true) {
        readTraceLine(&deadlockLine);
        if (deadlockLine == "Deadlock graph:") {
            break;
        }
    }

    // Add to the deadlock stuff.
    deadlockData << "<pre>" << endl
                 << deadlockLine << endl;

    // Scan and dump out the deadlock stuff.
    while (true) {
        if (readTraceLine(&deadlockLine)) {
            // Done yet?
            if ((deadlockLine.substr(0, 4) == "sess") ||
                (deadlockLine.substr(0, 4) == "Rows") ||
                (deadlockLine.substr(0, 4) == "----")) {
                break;
            }

            // Keep going, add to the dump.
            deadlockData << deadlockLine << endl;
        }
    }

    // Terminate the deadlock data.
    deadlockData << "</pre>";

    // Report the error in the report file.
    if (!mOptions->html()) {
        *mOfs << setw(MAXLINENUMBER) << currentLineNumber << ' '
              << setw(MAXLINENUMBER) << ' ' << ' '
              << setw(MAXLINENUMBER) << ' ' << ' '
              << setw(MAXLINENUMBER) << ' ' << ' '
              << setw(MAXLINENUMBER) << ' ' << ' '
              << deadlockData.str() << endl;
    } else {
        *mOfs << "<tr><td class=\"number\">" << currentLineNumber << "</td>"
              << "<td>" << "&nbsp;" << "</td>"
              << "<td>" << "&nbsp;" << "</td>"
              << "<td>" << "&nbsp;" << "</td>"
              << "<td>" << "&nbsp;" << "</td><td class=\"error_text\">"
              << deadlockData.str()
              << "</td></tr>" << endl;

    }

    // And on the command line.
    cout << "DEADLOCK detected at line: " << currentLineNumber << endl;

    // Scan the process state until the end.
    while (true) {
        readTraceLine(&deadlockLine);
        if (deadlockLine == "END OF PROCESS STATE") {
            break;
        }
    }

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parseDEADLOCK(): Exit." << endl;
    }
}




