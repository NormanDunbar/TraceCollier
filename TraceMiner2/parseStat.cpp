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

/** @file parseStat.cpp
 * @brief Implementation file for the tmTraceFile.parseSTAT() function.
 */

#include "tmtracefile.h"
#include "gnu.h"

#ifndef USE_REGEX
    #include "utilities.h"
#endif

/** @brief Parses a "STAT" line.
 *
 * @param thisLine const string&. The line of text with "STAT" in.
 * @return bool. Returns true if all ok. False otherwise.
 *
 * Parses a line from the trace file. The line is expected
 * to be the STAT \#cursor line.
 *
 * The tmCursor associated with this STAT is found, and the closed flag updated.
 * This is done because some cursors don't have a CLOSE after the various STATs.
 */
bool tmTraceFile::parseSTAT(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parseSTAT(" << mLineNumber << "): Entry." << endl;
    }

    // STAT #3074753576 id=1 ...
    string cursorID = "";
    bool matchOk = true;

#ifdef USE_REGEX
    regex reg("STAT\\s(#\\d+).*");
    smatch match;

    // Extract the cursorID.
    if (regex_match(thisLine, match, reg)) {
        cursorID = match[1];
    } else {
        matchOk = false;
    }
#else
    cursorID = getCursor(thisLine, &matchOk);
#endif  // USE_REGEX

    // Did it all work?
    if (!matchOk) {
        stringstream s;
        s << "parseSTAT(): Cannot match against STAT at line: "
          <<  mLineNumber << "." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parseSTAT(" << mLineNumber << "): Exit." << endl;
        }

        return false;
    }

    // Find the existing cursor.
    map<string, tmCursor *>::iterator i = findCursor(cursorID);

    // Not found? Don't care.
    if (i == mCursors.end()) {
        if (mOptions->verbose()) {
            *mDbg << "parseSTAT(" << mLineNumber << "): CursorID: " << cursorID << " - Not found. Exit." << endl;
        }

        return true;
    }

    // Found. Close it.
    if (!(i->second->isClosed())) {
        i->second->setClosed(true);
    }

    // Looks like a good stat.
    if (mOptions->verbose()) {
        *mDbg << "parseSTAT(" << mLineNumber << "): CursorID: "
              << cursorID << " - now/already closed." << endl
              << "parseSTAT(" << mLineNumber << "): Exit." << endl;
    }

    return true;
}




