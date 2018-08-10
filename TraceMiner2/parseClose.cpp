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

/** @file parseClose.cpp
 * @brief Implementation file for the tmTraceFile.parseCLOSE() function.
 */

#include "tmtracefile.h"
#include "gnu.h"

#ifndef USE_REGEX
    #include "utilities.h"
#endif

/** @brief Parses a "CLOSE" line.
 *
 * @param thisLine const string&. The line of text with "CLOSE" in.
 * @return bool. Returns true if all ok. False otherwise.
 *
 * Parses a line from the trace file. The line is expected
 * to be the CLOSE \#cursor line.
 *
 * The tmCursor associated with this CLOSE is found, and the closed flag updated.
 * Depth is ignored. A buig in Oracle, it seems, can PARSE at DEP=1 but CLOSE at DEP=0!
 */
bool tmTraceFile::parseCLOSE(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parseCLOSE(" << mLineNumber << "): Entry." << endl;
    }

    // CLOSE #4155332696:c=0,e=1,dep=0,type=3,tim=1039827725793
    string cursorID = "";
    unsigned depth = 0;
    bool matchOk = true;

#ifdef USE_REGEX
    regex reg("CLOSE\\s(#\\d+).*?dep=(\\d+).*");
    smatch match;

    // Extract the cursorID, the length and the depth.
    if (regex_match(thisLine, match, reg)) {
        cursorID = match[1];
        depth = stoul(match[2], NULL, 10);
    } else {
        matchOk = false;
    }
#else
    cursorID = getCursor(thisLine, &matchOk);
    if (matchOk) {
        depth = getDigits(thisLine, "dep=", &matchOk);
    }
#endif  // USE_REGEX

    // Did it all work?
    if (!matchOk) {
        stringstream s;
        s << "parseCLOSE(): Cannot match against CLOSE at line: "
          <<  mLineNumber << "." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parseCLOSE(): Exit." << endl;
        }

        return false;
    }

    // Find the existing cursor.
    map<string, tmCursor *>::iterator i = findCursor(cursorID);

    // Found?
    if (i != mCursors.end()) {
        // Yes. Thankfully! Update the closed flag.
        // i is an iterator to a pair<string, tmCursor *>
        // So i->first is the string.
        // And i->second is the tmCursor pointer.
        i->second->setClosed(true);
        // Issue #11: Binds line not cleared on close.
        i->second->setBindsLine(0);
    } else {
        // Not found. Oh dear! Flag it up if depth was zero.
        // Return true as if it worked to get
        // around an Oracle bug that I think I just found.

        stringstream s;
        if (depth <= mOptions->depth()) {
            s << "parseCLOSE(): Found CLOSE for cursor " << cursorID
              << " at line: " << mLineNumber
              << ", but not found in existing cursor list." << endl;
            cerr << s.str();
        }

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parseCLOSE(): Exit." << endl;
        }

        // We should be returning false if we don't find the cursor
        // but Oracle .... !
        //return false;
        return true;
    }

    // Looks like a good close.
    if (mOptions->verbose()) {
        *mDbg << "parseCLOSE(): Exit." << endl;
    }

    return true;
}



