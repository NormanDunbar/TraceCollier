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

/** @file parseParseError.cpp
 * @brief Implementation file for the tmTraceFile.parsePARSEERROR() function.
 */

#include "tmtracefile.h"
#include "utilities.h"


/** @brief Parses a "PARSE ERROR" line.
 *
 * @param thisLine const string&. The trace line with "PARSE ERROR" in.
 * @return bool. Returns true if all ok. False otherwise.
 *
 * Parses a line from the trace file. The line is expected
 * to be the PARSE ERROR \#cursor line.
 */
bool tmTraceFile::parsePARSEERROR(const string &thisLine) {

    // PARSE ERROR #4573797608:len=21 dep=0 uid=368 oct=3 lid=368 tim=39554896622951 err=923

    if (mOptions->verbose()) {
        *mDbg << "parsePARSEERROR(" << mLineNumber << "): Entry." << endl;
    }

    // Grab the first line of the failed SQL.
    // This will update mLineNumber.
    string nextLine;
    readTraceLine(&nextLine);

    // Trim off the unwanted stuff from the error line.
    string errorStuff = thisLine;
    string::size_type colonPos = 0;

    colonPos = errorStuff.find(":");
    if (colonPos != string::npos) {
        errorStuff = errorStuff.substr(0, colonPos-1);
    }

    // Extract the depth.
    bool ok = false;
    unsigned depth = getDigits(thisLine, "dep=", &ok);
    if (!ok) {
        depth = -1;
    }

    // Write the broken line to the report file.
    if (!mOptions->html()) {
        *mOfs << setw(MAXLINENUMBER) << ' ' << ' '
              << setw(MAXLINENUMBER) << mLineNumber-1 << ' '
              << setw(MAXLINENUMBER) << ' ' << ' '
              << setw(MAXLINENUMBER) << mLineNumber << ' '
              << setw(MAXLINENUMBER) << depth << ' '
              << thisLine << ' '
              << endl;
    } else {
        *mOfs << "<tr><td class=\"number\">" << "&nbsp;" << "</td>"
              << "<td class=\"number\">" << "&nbsp;" << mLineNumber-1 << "</td>"
              << "<td>" << "&nbsp;" << "</td>"
              << "<td class=\"number\">" << "&nbsp;" << mLineNumber << "</td>"
              << "<td class=\"number\">" << depth << "</td>"
              << "<td class=\"error_text\">" << errorStuff << "<br>"
              << nextLine << "</td></tr>" << endl;
    }

    // And on the command line.
    cout << "PARSE ERROR detected at line: " << mLineNumber << endl;


    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parsePARSEERROR(" << mLineNumber << "): Exit." << endl;
    }

    return true;
}


