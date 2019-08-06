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
#include "gnu.h"

#ifndef USE_REGEX
    #include "utilities.h"
#endif


/** @brief Parses an "ERROR" line.
 *
 * @param thisLine const string&. The line with "ERROR" in it.
 * @return bool. Returns true if all ok. False otherwise.
 *
 * Parses a line from the trace file. The line is expected
 * to be an ERROR \#cursor line.
 */
bool tmTraceFile::parseERROR(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parseERROR(" << mLineNumber << "): Entry." << endl;
    }

    // ERROR #275452960:err=31013 tim=1075688943194

    string cursorID = "";
    unsigned errorCode = 0;
    bool matchOk = true;

#ifdef USE_REGEX
    regex reg("ERROR\\s(#\\d+):err=(\\d+).*");
    smatch match;

    // Extract the cursorID and error code.
    if (regex_match(thisLine, match, reg)) {
        cursorID = match[1];
        errorCode = stoul(match[2], NULL, 10);
    } else {
        matchOk = false;
    }
#else
    cursorID = getCursor(thisLine, &matchOk);
    if (matchOk) {
        errorCode = getDigits(thisLine, "err=", &matchOk);
    }
#endif  // USE_REGEX

    // Did it all work?
    if (!matchOk) {
        stringstream s;
        s << "parseERROR(" << mLineNumber << "): Cannot match against ERROR at line: "
          <<  mLineNumber << "." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parseERROR(" << mLineNumber << "): Exit." << endl;
        }

        return false;
    }


    // This could be an error in recursive SQL, but check if we have
    // a cursor for the cursorID which would indicate user level SQL.
    map<string, tmCursor *>::iterator i = findCursor(cursorID);

    // If we found it, it must be depth <= depth().
    // Otherwise, quietly ignore it, it's recursive.
    string oraError = "ORA-" + std::to_string(errorCode);

    if (i != mCursors.end()) {
        unsigned temp = i->second->bindsLine();
        string bindsLine = std::to_string(temp);
        if (temp == 0) {
            bindsLine = "No binds";
        }

        // Report the error in the report file.
        // EXEC(ERROR) line numbers.
        if (!mOptions->html()) {
            *mOfs << setw(MAXLINENUMBER) << i->second->execLine() << '/' << mLineNumber << ' '
                  << setw(MAXLINENUMBER) << i->second->sqlParseLine() << ' '
                  << setw(MAXLINENUMBER) << bindsLine << ' '
                  << setw(MAXLINENUMBER) << i->second->sqlLineNumber() << ' '
                  << setw(MAXLINENUMBER) << ' ' << ' '
                  << " ERROR: " << oraError << endl;
        } else {
            *mOfs << "<tr><td class=\"number\">" << i->second->execLine() << '/' << mLineNumber << "</td>"
                  << "<td class=\"number\">" << i->second->sqlParseLine() << "</td>"
                  << "<td class=\"number\">" << bindsLine << "</td>"
                  << "<td class=\"number\">" << i->second->sqlLineNumber() << "</td>"
                  << "<td>" << "&nbsp;" << "</td>"
                  << "<td class=\"error_text\">"
                  << " ERROR: " << oraError
                  << "</td></tr>" << endl;

        }

        // And on the command line.
        cout << "ERROR " << oraError << " in Cursor: " << i->second->cursorId()
             << " detected at line: " << mLineNumber << endl;

    }

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parseERROR(" << mLineNumber << "): Exit." << endl;
    }

    return true;
}




