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

#include "tmtracefile.h"
#include "gnu.h"

#include "utilities.h"


/** @file tmtracefile.cpp
 * @brief Implementation file for the tmTraceFile object.
 */

/** @brief Constructor for a tmTraceFile object.
 *
 * @param options *tmOptions.
 *
 * Constructs a new tmTraceFile class, using the parsed command line options.
 */
tmTraceFile::tmTraceFile(tmOptions *options)
{
    mOriginalTraceFileName = "";
    mDatabaseVersion = "";
    mOracleHome = "";
    mInstanceName = "";
    mSystemName = "";
    mNodeName = "";
    mLineNumber = 0;
    mBatchCount = 0;
    mExecCount = -1;
    mUnprocessedLine.clear();
    mIfs = NULL;
    mOfs = NULL;
    mDbg = NULL;

    mOptions = options;
}


/** @brief Destructor for a tmTraceFile object.
 *
 */
tmTraceFile::~tmTraceFile()
{
    // Destructor.
    // If still open, close the trace file.
    cleanUp();
}


/** @brief Parses a complete trace file.
 *
 * @return bool.
 *
 * This function will parse an Oracle trace file and report on the SQL and
 * the various binds etc used in the EXEC lines found. Only 'DEP=0' EXECs are
 * considered.
 *
 * If the report file fails to open, consider that fatal. However, if the
 * debug file fails to open, just turn off verbose mode and try to carry on.
 *
 * Returns true to indicate success or false for a failure of some kind.
 */
bool tmTraceFile::parse()
{
    // We might need the debug file, but if we fail to open it, just carry on.
    if (mOptions->verbose()) {
        if (!openDebugFile()) {
            cerr << "TraceMiner2: Attempting to continue." << endl;
            mOptions->setVerbose(false);
        }
    }

    if (mOptions->verbose()) {
        *mDbg << "parse(): Entry." << endl;
    }

    // We need the report file here.
    if (!openReportFile()) {
        return false;
    }

    // Report file is open.
    // Reset the EXEC counter.
    mExecCount = 0;


    // Ready to go, lets parse a trace file.
    if (!openTraceFile()) {
        if (mOptions->verbose()) {
            *mDbg << "parse(): Cannot open trace file. Exit." << endl;
        }

        return false;
    }

    // Read in the header stuff, and attempt to validate this file
    // as an Oracle trace file. It could be something else!
    if (!parseHeader()) {
        cleanUp();

        if (mOptions->verbose()) {
            *mDbg << "parse(): parseHeader() failed. Error exit." << endl;
        }

        return false;
    }

    // File is open, header is parsed.
    if (!parseTraceFile()) {
        if (mOptions->verbose()) {
            *mDbg << "parse(): Cannot parse trace file. Exit." << endl;
        }

        return false;
    }

    // It was a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parse(): Exit." << endl;
    }

    // Close the table if HTML requested.
    if (mOptions->html()) {
        *mOfs << endl
              << "</table></body></html>" << endl;
    }

    return true;
}

/** @brief Parses a trace file.
 *
 * @return bool.
 *
 * This function will parse the main body of a trace file and report on the SQL and
 * the various binds etc used in the EXEC lines found. Only 'DEP=0' EXECs are
 * considered.
 *
 * Returns true to indicate success or false for a failure.
 */
bool tmTraceFile::parseTraceFile()
{
    // Process a trace file.
    string traceLine;
    string chunk;
    bool matchOk = true;

#ifdef USE_REGEX
    // Regex to extract the first command on the line.
    regex reg("(.*?)\\s#\\d+.*");
    smatch match;
#endif // USE_REGEX

    // The main parsing loop. What kind of line have we
    // read? Deal with it accordingly. If a parseBINDS() call
    // Read too far, process that line rather than reading another.
    while ((!mUnprocessedLine.empty()) ||
           readTraceLine(&traceLine)) {

        // Make sure we parse the unprocessed line from parseBINDS().
        if (!mUnprocessedLine.empty()) {
            traceLine = mUnprocessedLine;
            mUnprocessedLine.clear();
        }

        // Strip out those damned timestamp lines!
        if (traceLine.substr(0, 4) == "*** ")
        {
            if (mOptions->verbose()) {
                *mDbg << "parseTraceFile(): Ignoring timestamp line ["
                      << traceLine << ']' << endl;
            }

            continue;
        }

#ifdef USE_REGEX
        matchOk = regex_match(traceLine, match, reg);
        if (matchOk) {
            // Extract the command from the first grouping.
            chunk = match[1];
        }
#else
        // We can't use REGEX, so we have to do this the hard way!
        // Which means it complicates the order below!
        matchOk = true;
        chunk = traceLine.substr(0, 7);
#endif // USE_REGEX

        if (matchOk) {

            // PARSING IN CURSOR #cursorID
            if (chunk == "PARSING IN CURSOR" ||
                chunk == "PARSING") {
                if (!parsePARSING(traceLine)) {
                    return false;
                }
                continue;
            }

            // PARSE ERROR
            if (chunk == "PARSE ERROR" ||
                chunk == "PARSE E") {
                continue;
            }

            // PARSE #cursorID
            if (chunk == "PARSE" ||
                chunk == "PARSE #") {
                if (!parsePARSE(traceLine)) {
                    return false;
                }
                continue;
            }

            // BINDS #cursorID
            if (chunk == "BINDS" ||
                chunk == "BINDS #") {
                if (!parseBINDS(traceLine)) {
                    return false;
                }
                continue;
            }

            // CLOSE #cursorID
            if (chunk == "CLOSE" ||
                chunk == "CLOSE #") {
                if (!parseCLOSE(traceLine)) {
                    return false;
                }
                continue;
            }

            // STAT #cursorID
            if (chunk == "STAT" ||
                chunk.substr(0, 6) == "STAT #") {
                if (!parseSTAT(traceLine)) {
                    return false;
                }
                continue;
            }

            // ERROR #cursorID
            if (chunk == "ERROR" ||
                chunk == "ERROR #") {
                if (!parseERROR(traceLine)) {
                    return false;
                }
                continue;
            }

            if (chunk == "EXEC" ||
                chunk.substr(0, 6) == "EXEC #") {
                if (!parseEXEC(traceLine)) {
                    return false;
                }
                continue;
            }

            // We don't need the continue above, to be honest.
            // But if I ever add another check here, I'll probably
            // forget to add one in the check above. It's what I do! :(

        } else {

            // XCTEND (COMMIT/ROLLBACK).
            // Beware, there is no cursorID here, so no #.
            // So no regex_match()!
            if (traceLine.substr(0, 6) == "XCTEND") {
                if (!parseXCTEND(traceLine)) {
                    return false;
                }
                continue;
            }
        }
    }

    // We have a good parse.
    return true;
}


/** @brief Parses a trace file header.
 *
 * @return bool.
 *
 * Validates that this looks to be an Oracle trace file, then
 * reads in a few details from the header.
 */
bool tmTraceFile::parseHeader() {

    if (mOptions->verbose()) {
        *mDbg << "parseHeader(): Entry." << endl;
    }

    string traceLine;
    bool ok = readTraceLine(&traceLine);

    if (!ok) {
        stringstream s;
        s << "parseHeader(): Cannot read first line from "
          << mOptions->traceFile() << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parseHeader(): Error exit." << endl;
        }

        return false;
    }

    // Have we got a trace file? The first line starts "Trace file ..."
    if (traceLine.length() > 10) {
        string chunk = traceLine.substr(0, 10);
        if (!(chunk == "Trace file")) {
            stringstream s;
            s << mOptions->traceFile() << " is not an Oracle trace file." << endl
              << "Missing 'Trace file' in header." << endl;
            cerr << s.str();

            if (mOptions->verbose()) {
                *mDbg << s.str()
                      << "parseHeader(): Error exit." << endl;
            }

            return false;
        }
    } else {
        stringstream s;
        s << mOptions->traceFile() << " is not an Oracle trace file." << endl
          << "Missing 'Trace file' in header." << endl;
        cerr << s.str();

            if (mOptions->verbose()) {
                *mDbg << s.str()
                      << "parseHeader(): Error exit." << endl;
            }

        return false;
    }

    // We have a valid (hopefully!) trace file.
    mOriginalTraceFileName = traceLine.substr(11);

    while (true) {
        ok = readTraceLine(&traceLine);
        if (!ok) {
            stringstream s;
            s << "parseHeader(): Cannot read header line(s) from "
              << mOptions->traceFile() << endl;
            cerr << s.str();

            if (mOptions->verbose()) {
                *mDbg << s.str()
                      << "parseHeader(): Error exit." << endl;
            }

            break;
        }

        string chunk = traceLine.substr(0, 10);

        if (chunk == "Oracle Dat") {
            mDatabaseVersion = traceLine.substr(16);
            continue;
        }

        if (chunk == "ORACLE_HOM") {
            mOracleHome = traceLine.substr(14);
            continue;
        }

        if (chunk == "System nam") {
            mSystemName = traceLine.substr(13); // Hmm. TAB character here sometimes!
            continue;
        }

        if (chunk == "Node name:") {
            mNodeName = traceLine.substr(11);
            continue;
        }

        if (chunk == "Instance n") {
            mInstanceName = traceLine.substr(15);
            continue;
        }

        if (chunk == "==========") {
            break;
        }
    }

    if (mOptions->verbose()) {
        *mDbg << "parseHeader(): Exit." << endl;
    }

    return ok;
}


/** @brief Opens a trace file.
 *
 * @return bool.
 *
 * Open a trace file and calls parseHeader() to read in the header details.
 *
 * A return of true indicates success, false indicates some failure occurred.
 */
bool tmTraceFile::openTraceFile()
{
    string traceFileName = mOptions->traceFile();

    if (mOptions->verbose()) {
        *mDbg << "openTraceFile(): Entry." << endl
              << "openTraceFile(): Trace File: [" << traceFileName << ']' << endl;
    }

    mIfs = new ifstream(traceFileName);

    if (!mIfs->good()) {
        stringstream s;
        s << "openTraceFile(): Cannot open trace file "
          << traceFileName << endl;
        cerr << s.str();
        cleanUp();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "openTraceFile(): Error exit." << endl;
        }

        return false;
    }

    // Looks like a valid trace file.
    if (mOptions->verbose()) {
        *mDbg << "openTraceFile(): Exit." << endl;
    }

    return true;

}


/** @brief Opens a debug file.
 *
 * @return bool.
 *
 * Open a debug file, if required.
 *
 * A return of true indicates success, false indicates some failure occurred.
 */
bool tmTraceFile::openDebugFile()
{
    string debugFileName = mOptions->debugFile();

    if (debugFileName.empty()) {
        // Very unlikely.
        return false;
    }

    mDbg = new ofstream(debugFileName);

    if (!mDbg->good()) {
        // Don't clean up if this fails to open. We ignore it later.
        cerr << "TraceMiner2: Cannot open debug file "
             << debugFileName << endl;
        return false;
    }

    // Allow thousands separator.
    // Affects *every* number >= 1000 sent to *mDbg.
    // Not sure I need the ',' (or whatever else I've put there!
    // Yup, the comma is not required now. Should be locale dependent.
    mDbg->imbue(locale(mDbg->getloc(), new ThousandsSeparator<char>(',')));

    // Looks like a valid debug file.
    return true;
}


/** @brief Opens a report file.
 *
 * @return bool.
 *
 * Open a report file.
 *
 * A return of true indicates success, false indicates some failure occurred.
 */
bool tmTraceFile::openReportFile()
{
    string reportFileName = mOptions->reportFile();

    if (mOptions->verbose()) {
        *mDbg << "openReportFile(): Entry." << endl
              << "Report File: [" << reportFileName << ']' << endl;
    }

    mOfs = new ofstream(reportFileName);

    if (!mOfs->good()) {
        stringstream s;
        s << "TraceMiner2: Cannot open report file "
          << reportFileName << endl;
        cerr << s.str();
        cleanUp();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "openReportFile(): Error exit." << endl;
        }

        return false;
    }

    // Allow thousands separator.
    // Affects *every* number >= 1000 sent to *mOfs.
    // Not sure I need the ',' (or whatever else I've put there!
    // Yup, the comma is not required now. Should be locale dependent.
    mOfs->imbue(locale(mOfs->getloc(), new ThousandsSeparator<char>(',')));

    reportHeadings();

    // Looks like a valid report file.
    if (mOptions->verbose()) {
        *mDbg << "openReportFile(): Exit." << endl;
    }

    return true;
}


/** @brief Writes the headings to the report file.
 *
 * If we hit a magic number of EXEC statements, we will throw
 * a new set of headings in the report file. This is useful as
 * large traces may require the reader to scroll up and down to
 * determine what the numbers in the report lines actually  are!
 *
 */
void tmTraceFile::reportHeadings() {

    if (mOptions->verbose()) {
        *mDbg << "reportHeadings(): EXEC count: " << mExecCount << " Entry." << endl;
    }

    if (!mOptions->html()) {
        // Headings. (TEXT)

        // Once per report file headings.
        if (mExecCount == -1) {
            *mOfs << "TraceMiner2" << endl
                  << "-----------" << endl << endl;

            *mOfs << "Processing Trace file: " << mOptions->traceFile() << endl << endl;
        }

        // Break up the report every MAXEXECS EXEC statements.
        if (mExecCount > 0) {
            *mOfs << endl << endl;
        }

        *mOfs << setw(MAXLINENUMBER) << "EXEC Line" << ' '
              << setw(MAXLINENUMBER) << "PARSE Line" << ' '
              << setw(MAXLINENUMBER) << "BINDS Line" << ' '
              << setw(MAXLINENUMBER) << "SQL Line" << ' '
              << setw(MAXLINENUMBER) << "DEP" << ' '
              << "SQL Text"
              << endl
              << setw(200) << setfill('-') << '-'
              << setfill(' ') << endl;
    } else {
        // Headings. (HTML)
        // Do we need to do the HTML headings at the very start?
        if (mExecCount == -1) {
            *mOfs << "<html lang=\"en\"><head>" << endl
                  << "<title>TraceMiner 2</title>" << endl
                  << "<meta charset=\"UTF-8\" />" << endl
                  << "<meta name=\"generator\" content=\"TraceMiner2\" />" << endl
                  << "<meta name=\"author\" content=\"Norman (at) Dunbar-it (dot) co (dot) uk\" />" << endl
                  << "<link rel=\"stylesheet\" href=\""
                  << fileName(mOptions->cssFileName()) << "\" />" << endl
                  << "<link rel=\"icon\" href=\"favicon.ico\" type=\"image/x-icon\" />"
                  << "</head>" << endl
                  << "<body>" << endl
                  << "<H1>TraceMiner2</H1>" << endl
                  << "<p><strong>Processing Trace File:</strong> "
                  << mOptions->traceFile() << "</p>" << endl;
        }

        // Are we in the middle of a trace? Start a new table if necessary.
        // This makes reading the report a lot easier.
        if (mExecCount > 0) {
            *mOfs << "</table>" << endl
                  << "<p>&nbsp;</p>" << endl << endl;
        }

        // I'm a little p1553d off at the WWW for HTML5's inability
        // to use CSS to specify a table or cell width in the CSS
        // file. This is a bad thing as I have to recompile if I need
        // to change the column widths.
        *mOfs << "<table style=\"width:95%\";>" << endl
              << "<tr><th style=\"width:6%\";>EXEC Line</th>"
              << "<th style=\"width:6%\";>PARSE Line</th>"
              << "<th style=\"width:6%\";>BINDS# Line</th>"
              << "<th style=\"width:6%\";>SQL Line</th>"
              << "<th style=\"width:3%\";>DEP</th>"
              << "<th>SQL Text</th></tr>"
              << endl;
    }

    if (mOptions->verbose()) {
        *mDbg << "reportHeadings(): EXEC count: " << mExecCount << " Exit." << endl;
    }

}


/** @brief Reads a single line from a trace file.
 *
 * @param aLine string*. Pointer to a string to receive a single line read from the trace file.
 * @return bool
 *
 * Reads a single line from a trace file. Updates the current
 * line number within the trace file.
 * Returns true if we are still good for more reading, false otherwise.
 */
bool tmTraceFile::readTraceLine(string *aLine) {

    while (true && mIfs->good()) {
        getline(*mIfs, *aLine);
        mLineNumber++;
        mBatchCount++;

        // Give some feedback on big trace files.
        if (mBatchCount == mOptions->feedBack()) {
            cerr << "readTraceLine(): " << mLineNumber << " lines read so far..."
                 << endl;
            mBatchCount = 0;
        }

        // We ignore empty lines, EOF() etc.
        if (!aLine->empty()) {
            break;
        }
    }

    // Verbose?
    if (mOptions->verbose()) {
        *mDbg << "readTraceLine(" << mLineNumber << "): [" << *aLine << "]"
              << " EOF = " << std::boolalpha << mIfs->eof() << endl;
    }

    return mIfs->good();
}


/** @brief Finds a cursor in the cursor list.
 *
 * @param cursorID const std::string&. The cursor we are looking for.
 * @return map<string, tmCursor *>::iterator. An iterator to the desired cursor, or if not found, a pointer to the end.
 *
 * Searches the mCursors map for a given key.
 * Returns a valid iterator if all ok, otherwise returns mCursors.end().
 */
map<string, tmCursor *>::iterator tmTraceFile::findCursor(const string &cursorID) {

    // Find an existing cursor in the map.

    if (mOptions->verbose()) {
        *mDbg << "findCursor(): Entry." << endl
              << "findCursor(): Looking for cursor: " << cursorID << endl;
    }

    map<string, tmCursor *>::iterator i = mCursors.find(cursorID);

    if (mOptions->verbose()) {
        if (i != mCursors.end()) {
            // Not found.
            *mDbg << "findCursor(): Cursor: " << cursorID
                  << " found. (SQL on line: " << i->second->sqlLineNumber()
                  << ')' << endl;
        } else {
            // Not found.
            *mDbg << "findCursor(): Cursor: " << cursorID
                  << " not found." << endl;
        }
    }

    if (mOptions->verbose()) {
        *mDbg << "findCursor(): Exit." << endl;
    }

    return i;
}


/** @brief Cleans up the cursor in the event that something was wrong.
 *
 * In the event that a parse goes badly, this function will be called
 * to clean up whatever mess there is, lying around in the tmTraceFile
 * object.
 */
void tmTraceFile::cleanUp() {
    // If still open, close the trace/output/debug files.
    if (mIfs) {
        if (mIfs->is_open()) {
            mIfs->close();
        }

        delete mIfs;
        mIfs = NULL;
    }

    if (mOfs) {
        if (mOfs->is_open()) {
            mOfs->close();
        }

        delete mOfs;
        mOfs = NULL;
    }

    // If we have any cursors, clean them out.
    //Beware, clear() doesn't destruct classes!
    if (mCursors.size()) {
        for (map<string, tmCursor *>::iterator i = mCursors.begin(); i != mCursors.end(); ++i) {
            if (mOptions->verbose()) {
                *mDbg << endl << "cleanUP(): Freeing cursor: " << i->second->cursorId() << endl;
                *mDbg << *(i->second);
            }

            // Destruct the tmCursor.
            delete i->second;
        }

        // Finally, clear the map.
        mCursors.clear();
    }

    // we must do this last of all, or the above might blow up!
    if (mDbg) {
        if (mDbg->is_open()) {
            mDbg->close();
        }

        delete mDbg;
        mDbg = NULL;
    }

}


