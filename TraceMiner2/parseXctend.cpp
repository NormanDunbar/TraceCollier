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

/** @file parseXctend.cpp
 * @brief Implementation file for the tmTraceFile.parseXCTEND() function.
 */

#include "tmtracefile.h"
#include "gnu.h"

#ifndef USE_REGEX
    #include "utilities.h"
#endif

/** @brief Parses a "XCTEND" line.
 *
 * @param thisLine const string&. The trace line with "XCTEND" in it.
 * @return bool. Returns true if all ok. False otherwise.
 *
 * Parses a line from the trace file. The line is expected
 * to be the XCTEND line indicating COMMIT or ROLLBACK.
 * It should be noted that this line has no cursor ID.
 */
bool tmTraceFile::parseXCTEND(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parseXCTEND(" << mLineNumber << "): Entry." << endl;
    }

    // XCTEND rlbk=0, rd_only=0, tim=524545341395

    bool matchOK = true;
    unsigned rollBack = 0;
    unsigned readOnly = 0;

#ifdef USE_REGEX
    regex reg("XCTEND\\srlbk=(\\d+).*?rd_only=(\\d+).*");
    smatch match;

    // Extract the rollback and read only flags.
    if (regex_match(thisLine, match, reg)) {
        rollBack = stoul(match[1], NULL, 10);
        readOnly = stoul(match[2], NULL, 10);
    } else {
        matchOK = false;
    }


#else
    // Find Rollback & Read Only indicators.
    rollBack = getDigits(thisLine, "rlbk=", &matchOK);
    if (matchOK) {
        readOnly = getDigits(thisLine, "rd_only=", &matchOK);
    }
#endif  // USE_REGEX

    if (!matchOK) {
        stringstream s;
        s << "parseXCTEND(): Cannot match XCTEND at line: "
          <<  mLineNumber << " - Trace file corrupt?" << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parseXCTEND(): Exit." << endl;
        }

        return false;
    }

    if (!mOptions->html()) {
        *mOfs << setw(MAXLINENUMBER) << mLineNumber << ' '
              << setw(MAXLINENUMBER) << ' ' << ' '
              << setw(MAXLINENUMBER) << ' ' << ' '
              << setw(MAXLINENUMBER) << ' ' << ' '
              << (rollBack ? "ROLLBACK " : "COMMIT ")
              << (readOnly ? "(Read Only)" : "(Read Write)") << endl;
    } else {
        *mOfs << "<tr><td class=\"number\">" << mLineNumber << "</td>"
              << "<td>" << "&nbsp;" << "</td>"
              << "<td>" << "&nbsp;" << "</td>"
              << "<td>" << "&nbsp;" << "</td><td class=\"text\">"
              << (rollBack ? "ROLLBACK " : "COMMIT ")
              << (readOnly ? "(Read Only)" : "(Read Write)")
              << "</td></tr>" << endl;
    }

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parseXCTEND(): Exit." << endl;
    }

    // Increment  the EXEC count for a COMMIT or ROLLBACK.
    mExecCount++;
    return true;
}

