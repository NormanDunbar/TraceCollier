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

/** @file parseExec.cpp
 * @brief Implementation file for the tmTraceFile.parseEXEC() function.
 */

#include "tmtracefile.h"
#include "gnu.h"

#ifndef USE_REGEX
    #include "utilities.h"
#endif

/** @brief Parses a "EXEC" line.
 *
 * @param thisLine const string&. Trace file line containing EXEC.
 * @return bool. Returns true if all ok. False otherwise.
 *
 * Parses a line from the trace file. The line is expected
 * to be the EXEC \#cursor line.
 *
 * The tmCursor associated with this PARSE is found, and its bind values
 * are extracted and merged into the SQL statement ready for output to
 * the report file.
 */
bool tmTraceFile::parseEXEC(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parseEXEC(" << mLineNumber << "): Entry. EXEC Count so far: " << mExecCount << endl;
    }

    // EXEC #5924310096:c=0,e=31,p=0,cr=0,cu=0,mis=0,r=0,dep=0,og=4,plh=1388734953,tim=526735705392
    string cursorID = "";
    unsigned depth = 0;
    bool matchOk = true;

#ifdef USE_REGEX
    regex reg("EXEC\\s(#\\d+).*?dep=(\\d+).*");
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
        s << "parseEXEC(" << mLineNumber << "): Cannot match against EXEC at line: "
          <<  mLineNumber << "." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str() << endl
                  << "parseEXEC(" << mLineNumber << "): Exit." << endl;
        }

        return false;
    }

    // We only care about user level SQL, so only depth <= depth().
    if (depth > mOptions->depth()) {
        // Ignore this one.
        if (mOptions->verbose()) {
            *mDbg << "parseEXEC(" << mLineNumber << "): Ignoring EXEC with dep=" << depth << '.' << endl
                  << "parseEXEC(" << mLineNumber << "): Exit." << endl;
        }

        return true;
    }

    // Find the cursor for this exec.
    map<string, tmCursor *>::iterator i = findCursor(cursorID);
    if (i == mCursors.end()) {
        stringstream s;
        s << "parseEXEC(" << mLineNumber << "): Cursor " << cursorID << " not found." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str() << endl
                  << "parseEXEC(" << mLineNumber << "): Exit." << endl;
        }

        return false;
    }

    // We have a depth <= depth() EXEC with a valid cursor, increment the EXEC counter.
    // And check if we need a fresh set of report headings?
    if ((mExecCount > mOptions->maxExecs())) {
        // Throw a new set of headings to make reading easier.
        reportHeadings();
        mExecCount = 0;
    }

    // Get the SQL Statement & binds.
    tmCursor *thisCursor = i->second;
    string sqlText = thisCursor->sqlText();

    // Find the binds map for this cursor.
    map<unsigned, tmBind *> *binds = thisCursor->binds();

    // Replace all the bind names we find, with the bind value.
    // I would love to be able to do this by finding the offset and
    // length of the bind name, at parse time for the SQL, but it
    // seems either that it is not possible, or my ability to use the STL
    // is lacking in skill. I suspect the latter!
    for (map<unsigned, tmBind *>::iterator i = binds->begin();
         i != binds->end();
         i++)
    {
        string::size_type bindPos = sqlText.find(i->second->bindName());
        if (bindPos != string::npos) {
            sqlText.replace(bindPos, i->second->bindName().length(), i->second->bindValue());
            if (mOptions->verbose()) {
                *mDbg << "parseEXEC(" << mLineNumber << "): Cursor: " << cursorID << ": Bind #"
                      << i->second->bindId() << ": Replacing: ["
                      << i->second->bindName() << "] with ["
                      << i->second->bindValue() << ']' << endl;
            }
        } else {
            // Hmm. This should never happen!
            stringstream s;
            s << "parseEXEC(" << mLineNumber << "): Cannot find '" << i->second->bindName() << " in ["
              << sqlText << "]." << endl;
            cerr << s.str();

            if (mOptions->verbose()) {
                *mDbg << s.str()
                      << "parseEXEC(" << mLineNumber << "): Exit." << endl;
            }

            return false;
        }
    }

    // And write the replaced SQL to the report file. If there are EXECs
    // with no PARSE then they have been EXECuted from session cached cursors.
    // The previous CLOSE for the cursor will be a TYP=1, 2 or 3. Only TYP=0
    // get a PARSE line.
    // Likewise, if there is a BINDS line of zero, it has no binds.
    string parseLineText = std::to_string(thisCursor->sqlParseLine());
    parseLineText = (parseLineText == "0") ? "From cache" : parseLineText;

    string parseClass = (parseLineText == "From cache") ? "text" : "number" ;

    string bindsLineText = std::to_string(thisCursor->bindsLine());
    bindsLineText = (bindsLineText == "0") ?  "No binds" : bindsLineText;

    string bindsClass = (bindsLineText == "No binds") ? "text" : "number" ;

    if (!mOptions->html()) {
        *mOfs << setw(MAXLINENUMBER) << mLineNumber << ' '
              << setw(MAXLINENUMBER) << parseLineText << ' '
              << setw(MAXLINENUMBER) << bindsLineText << ' '
              << setw(MAXLINENUMBER) << thisCursor->sqlLineNumber() << ' '
              << setw(MAXLINENUMBER) << depth << ' '
              << sqlText << ' '
              << endl;
    } else {
        *mOfs << "<tr><td class=\"number\">" << mLineNumber << "</td>"
              << "<td class=\"" << parseClass << "\">" << parseLineText << "</td>"
              << "<td class=\"" << bindsClass << "\">" << bindsLineText << "</td>"
              << "<td class=\"number\">" << thisCursor->sqlLineNumber() << "</td>"
              << "<td class=\"number\">" << depth << "</td>"
              << "<td class=\"text\"><pre>" << sqlText << "</pre></td></tr>"
              << endl;
    }

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parseEXEC(" << mLineNumber << "): Exit." << endl;
    }

    mExecCount++;
    return true;
}

