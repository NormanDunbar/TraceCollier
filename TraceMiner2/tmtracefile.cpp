#include "tmtracefile.h"

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

    // Regex to extract the first command on the line.
    regex reg("(.*?)\\s#\\d+.*");
    smatch match;

    // The main parsing loop. What kind of line have we
    // read? Deal with it accordingly.
    while (readTraceLine(&traceLine)) {

        if (regex_match(traceLine, match, reg)) {

            // Extract the command from the first grouping.
            string chunk = match[1];

            // PARSING IN CURSOR #cursorID
            if (chunk == "PARSING IN CURSOR") {
                if (!parsePARSING(traceLine)) {
                    return false;
                }
                continue;
            }

            // PARSE ERROR
            if (chunk == "PARSE ERROR") {
                continue;
            }

            // PARSE #cursorID
            if (chunk == "PARSE") {
                if (!parsePARSE(traceLine)) {
                    return false;
                }
                continue;
            }

            // BINDS #cursorID
            if (chunk == "BINDS") {
                continue;
            }

            // CLOSE #cursorID
            if (chunk == "CLOSE") {
                // At present, we no longer care about CLOSEing a cursor.
                continue;
            }

            // ERROR #cursorID
            if (chunk == "ERROR") {
                continue;
            }

            if (chunk == "EXEC") {
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
        cerr << "parseHeader(): Cannot read first line from "
             << mOptions->traceFile() << endl;

        if (mOptions->verbose()) {
            *mDbg << "parseHeader(): Cannot read first line from "
                  << mOptions->traceFile() << endl
                  << "parseHeader(): Error exit." << endl;
        }

        return false;
    }

    // Have we got a trace file? The first line starts "Trace file ..."
    if (traceLine.length() > 10) {
        string chunk = traceLine.substr(0, 10);
        if (!(chunk == "Trace file")) {
            cerr << mOptions->traceFile() << " is not an Oracle trace file." << endl;
            cerr << "Missing 'Trace file' in header." << endl;

            if (mOptions->verbose()) {
                *mDbg << "parseHeader(): Not an Oracle trace file. " << endl
                      << "parseHeader(): Error exit." << endl;
            }

            return false;
        }
    } else {
        cerr << mOptions->traceFile() << " is not an Oracle trace file." << endl;
        cerr << "Missing 'Trace file' in header." << endl;

            if (mOptions->verbose()) {
                *mDbg << "parseHeader(): Not an Oracle trace file. " << endl
                      << "parseHeader(): Error exit." << endl;
            }

        return false;
    }

    // We have a valid (hopefully!) trace file.
    // Extract heading details.
    // Position to the first real line.
    mOriginalTraceFileName = traceLine.substr(11);

    while (true) {
        ok = readTraceLine(&traceLine);
        if (!ok) {
            cerr << "parseHeader(): Cannot read header line(s) from "
                 << mOptions->traceFile() << endl;
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
              << "Trace File: [" << traceFileName << ']' << endl;
    }

    mIfs = new ifstream(traceFileName);

    if (!mIfs->good()) {
        cerr << "TraceMiner2: Cannot open trace file "
             << traceFileName << endl;
        cleanUp();

        if (mOptions->verbose()) {
            *mDbg << "openTraceFile(): Error exit." << endl;
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
        cerr << "TraceMiner2: Cannot open report file "
             << reportFileName << endl;
        cleanUp();

        if (mOptions->verbose()) {
            *mDbg << "openReportFile(): Error exit." << endl;
        }

        return false;
    }

    // Allow thousands separator.
    // Affects *every* number >= 1000 sent to *mOfs.
    mOfs->imbue(locale(mOfs->getloc(), new ThousandsSeparator<char>(',')));

    // Looks like a valid report file.
    if (mOptions->verbose()) {
        *mDbg << "openReportFile(): Exit." << endl;
    }

    return true;
}


/** @brief Reads a single line from a trace file.
 *
 * @param aLine *std::string. Pointer to a string to receive a single line read from the trace file.
 * @return bool
 *
 * Reads a single line from a trace file. Updates the current
 * line number within the trace file.
 * Returns true if we are still good for more reading, false otherwise.
 */
bool tmTraceFile::readTraceLine(string *aLine) {

    getline(*mIfs, *aLine);
    mLineNumber++;

    // Verbose?
    if (mOptions->verbose()) {
        *mDbg << "readTraceLine(" << mLineNumber << "): [" << *aLine << "]"
              << " EOF = " << std::boolalpha << mIfs->eof() << endl;
    }

    return mIfs->good();
}


/** @brief Parses a "PARSING IN CURSOR" line.
 *
 * @param thisLine &std::string. A reference to a single line from the tarce file. This
 *        will always be the "PARSING IN CURSOR" line.
 * @return bool.
 *
 * Parses a line form the trace file. The line is expected
 * to be the PARSING IN CURSOR line. We are only interested in
 * cursors at depth = 0.
 *
 * Creates a tmCursor object and appends it to the map<string, tmCursor *>
 * that we use to hold these things, if the depth is zero.
 *
 * Returns true if all ok. False otherwise.
 */
bool tmTraceFile::parsePARSING(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parsePARSING(): Entry." << endl;
    }

    // PARSING IN CURSOR #4572676384 len=229 dep=1 ...
    regex reg("PARSING IN CURSOR\\s(#\\d+)\\slen=(\\d+)\\sdep=(\\d+).*");
    smatch match;

    std::stringstream ss;

    // Extract the cursorID, the length and the depth.
    if (!regex_match(thisLine, match, reg)) {
        cerr << "parsePARSING(): Cannot match regex against PARSING IN CURSOR at line: "
             <<  mLineNumber << "." << endl;
        return false;
    }

    // Found it!
    // Extract the goodies! We definitely have all three matches.
    // Match[0] = "#12345678 len=123 dep=0"
    // Match[1] = "#12345678"
    // Match[2] = "123"
    // Match[3] = "0"

    // No validation necessary, they are digits etc, as required.

    // The SQL starts on the following line, not this one!
    unsigned sqlLine = mLineNumber + 1;
    string cursorID = match[1];
    unsigned sqlLength = stoul(match[2], NULL, 10);
    unsigned depth = stoul(match[3], NULL, 10);

    // We only care about user level SQL so depth 0 only gets saved away.
    // This does mean that SQL executed in a PL/SQL call is ignored though.
    // So far, that's what my customer(s) want. (Hello Rich.)
    if (depth) {
        // Nothing to do here.
        return true;
    }

    tmCursor *thisCursor = new tmCursor(cursorID, sqlLength, sqlLine);
    if (!thisCursor) {
        cerr << "parsePARSING(): Cannot allocate a new tmCursor." << endl;

        if (mOptions->verbose()) {
            *mDbg << "parsePARSING(): Cannot allocate a new tmCursor." << endl
                  << "parsePARSING(): Exit." << endl;
        }

        return false;
    }

    // Extract the SQL Text into a stream. This handles end of line for us.

    string aLine;

    while (readTraceLine(&aLine)) {
        if (aLine.substr(0, 11) == "END OF STMT") {
            break;
        }

        // Append this new SQL line to the existing lines.
        if (!ss.str().empty()) {
            // Append a system dependent newline to the current text.
            ss << endl;
        }
        // And append this new line of text.
        ss << aLine;
    }

    // An iterator for the insert into the map. AKA where are we?
    pair<map<string, tmCursor *>::iterator, bool> exists;

    // Stash this new cursor. If the cursor exists, update it. Maps are weird!
    // CusrorIDs are like Highlanders. There can be only one! ;)
    exists = mCursors.insert(pair<string, tmCursor *>(cursorID, thisCursor));

    // If the bool is false, the insert failed - already there.
    // So we simply update.
    //
    // Exists is a pair<mCursors::iterator, bool>. It indexes the map at the
    // position of the inserted or already existing cursor data.
    // Exists.second is the bool. True=inserted, False=already exists.
    // Exists.first is an iterator (pseudo pointer) to a pair <string, tmCursor *>.
    // Exists.first->first is the string, aka the cursorID.
    // Exists.first->second is the tmCursor pointer.
    //
    // Phew!

    // So, after all that, did we insert or find our cursor?
    if (!exists.second) {
        // Update existing cursor details. Only the
        // SQL details will have changed. At the moment.
        // And we have not yet parsed this SQL text.
        exists.first->second->setSQLLineNumber(sqlLine);
        exists.first->second->setSQLLength(sqlLength);
        exists.first->second->setSQLParseLine(0);
        delete thisCursor;
    }

    // Then set the SQL Text, regardless.
    exists.first->second->setSQLText(ss.str());

    // Verbose?
    if (mOptions->verbose()) {
        *mDbg << endl << "parsePARSING(): Creating Cursor: "
              << exists.first->first << endl
              << *(exists.first->second);
    }

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parsePARSING(): Exit." << endl;
    }

    return true;

}

/** @brief Parses a "PARSE" line.
 *
 * Parses a line from the trace file. The line is expected
 * to be the PARSE #cursor line.
 *
 * The tmCursor associated with this PARSE is found, and updated to the new
 * source file line number. Only the most recent PARSE is stored for each tmCursor.
 *
 * Returns true if all ok. False otherwise.
 */
bool tmTraceFile::parsePARSE(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parsePARSE(): Entry." << endl;
    }

    // PARSE #4572676384 ... dep=1 ...
    regex reg("PARSE\\s(#\\d+).*?dep=(\\d+).*");
    smatch match;

    // Extract the cursorID, the length and the depth.
    if (!regex_match(thisLine, match, reg)) {
        cerr << "parsePARSE(): Cannot match regex against PARSE # at line: "
             <<  mLineNumber << "." << endl;

        if (mOptions->verbose()) {
            *mDbg << "parsePARSE(): Cannot match regex against PARSE # at line: "
                  <<  mLineNumber << "." << endl
                  << "parsePARSE(): Exit." << endl;
        }

        return false;
    }

    // Found it!
    // Extract the goodies! We definitely have all three matches.
    // Match[0] = the whole line!
    // Match[1] = "#12345678"
    // Match[2] = "0"

    // No validation necessary, they are digits etc, as required.

    // The SQL starts on the following line, not this one!
    string cursorID = match[1];
    unsigned depth = stoul(match[2], NULL, 10);

    // We only care about user level SQL, so only depth 0.
    if (!depth) {
        // Find the existing cursor.
        map<string, tmCursor *>::iterator i = findCursor(cursorID);

        // Found?
        if (i != mCursors.end()) {
            // Yes. Thankfully! Update the PARSE line number.
            // i is an iterator to a pair<string, tmCursor *>
            // So i->first is the string.
            // And i->second is the tmCursor pointer.
            i->second->setSQLParseLine(mLineNumber);
        } else {
            // Not found. Oh dear!
            cerr << "parsePARSE(): Found PARSE for cursor " << cursorID
                 << " but not found in existing cursor list." << endl;

            if (mOptions->verbose()) {
                *mDbg << "parsePARSE(): Found PARSE for cursor " << cursorID
                      << " but not found in existing cursor list." << endl
                      << "parsePARSE(): Exit." << endl;
            }

            return false;
        }
    }

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parsePARSE(): Exit." << endl;
    }

    return true;
}



/** @brief Parses a "BINDS" line.
 *
 * Parses a line from the trace file. The line is expected
 * to be the BINDS #cursor line.
 *
 * This function extracts the values for all listed binds
 * for the given cursor. The values are used to update
 * the binds map member of the appropriate tmCursor object.
 *
 * Returns true if all ok. False otherwise.
 */
bool tmTraceFile::parseBINDS(const string &thisLine) {

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parseBINDS(): Exit." << endl;
    }

    return true;
}


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


/** @brief Parses a "XCTEND" line.
 *
 * Parses a line from the trace file. The line is expected
 * to be the XCTEND line indicating COMMIT or ROLLBACK.
 * It should be noted that this line has no cursor ID.
 *
 * Returns true if all ok. False otherwise.
 */
bool tmTraceFile::parseXCTEND(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parseXCTEND(): Entry." << endl;
    }

    // XCTEND rlbk=0, rd_only=0, tim=524545341395
    regex reg("XCTEND\\srlbk=(\\d+).*?rd_only=(\\d+).*");
    smatch match;

    // Extract the rollback and read only flags.
    if (!regex_match(thisLine, match, reg)) {
        cerr << "parseXCTEND(): Cannot match regex against XCTEND at line: "
             <<  mLineNumber << "." << endl;

        if (mOptions->verbose()) {
            *mDbg << "parseXCTEND(): Cannot match regex against XCTEND at line: "
                  <<  mLineNumber << "." << endl
                  << "parseXCTEND(): Exit." << endl;
        }

        return false;
    }

    unsigned rollBack = stoul(match[1], NULL, 10);
    unsigned readOnly = stoul(match[2], NULL, 10);

    // This is temporary *****************************
    mOfs->width(MAXLINENUMBER);
    *mOfs << mLineNumber << ' ';

    if (!rollBack) {
        *mOfs << "COMMIT: ";
    } else {
        *mOfs << "ROLLBACK: ";
    }
    *mOfs <<  (readOnly ? "(Read Only)" : "(Read Write)") << endl;
    // This is temporary *****************************

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parseXCTEND(): Exit." << endl;
    }

    return true;
}


/** @brief Parses an "ERROR" line.
 *
 * Parses a line from the trace file. The line is expected
 * to be an ERROR #cursor line.
 *
 * Returns true if all ok. False otherwise.
 */
bool tmTraceFile::parseERROR(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parseERROR(): Entry." << endl;
    }

    // ERROR #275452960:err=31013 tim=1075688943194
    regex reg("ERROR\\s(#\\d+):err=(\\d+).*");
    smatch match;

    // Extract the cursorID and error code.
    if (!regex_match(thisLine, match, reg)) {
        cerr << "parseERROR(): Cannot match regex against ERROR at line: "
             <<  mLineNumber << "." << endl;

        if (mOptions->verbose()) {
            *mDbg << "parseERROR(): Cannot match regex against ERROR at line: "
                  <<  mLineNumber << "." << endl
                  << "parseERROR(): Exit." << endl;
        }

        return false;
    }

    string cursroID = match[1];
    unsigned errorCode = stoul(match[2], NULL, 10);

    // This is temporary *****************************
    mOfs->width(MAXLINENUMBER);
    *mOfs << mLineNumber
          << " ERROR: ORA-" << errorCode << endl;
    // This is temporary *****************************


    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parseERROR(): Exit." << endl;
    }

    return true;
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
                  << " found." << endl;
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
            mCursors.erase(i);
            delete i->second;
        }
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


