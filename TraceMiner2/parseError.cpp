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

/** @file parseError.cpp
 * @brief Implementation file for the tmTraceFile.parseERROR() function.
 */

#include "tmtracefile.h"



/** @brief Parses an "ERROR" line.
 *
 * @param thisLine const string&. The line with "ERROR" in it.
 * @return bool. Returns true if all ok. False otherwise.
 *
 * Parses a line from the trace file. The line is expected
 * to be an ERROR #cursor line.
 */
bool tmTraceFile::parseERROR(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parseERROR(" << mLineNumber << "): Entry." << endl;
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
        if (mOptions->html()) {
            *mOfs << setw(MAXLINENUMBER) << mLineNumber << ' '
                  << setw(MAXLINENUMBER) << ' ' << ' '
                  << setw(MAXLINENUMBER) << ' ' << ' '
                  << " ERROR: ORA-" << errorCode << endl;
        } else {
            *mOfs << "<tr><td class=\"number\">" << mLineNumber << "</td>"
                  << "<td>" << "&nbsp;" << "</td>"
                  << "<td>" << "&nbsp;" << "</td><td class=\"text\">"
                  << " ERROR: ORA-" << errorCode
                  << "</td></tr>" << endl;

        }
    }

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parseERROR(): Exit." << endl;
    }

    return true;
}




