#include "tmtracefile.h"

/** @file tmtracefile.cpp
 * @brief Implementation file for the tmTraceFile object.
 */

/** @brief Cleans up the cursor in the event that something was wrong.
 *
 * In the event that a parse goes badly, this function will be called
 * to clean up whatever mess there is, lying around in the tmTraceFile
 * object.
 */
void tmTraceFile::cleanUp() {
    // If still open, close the trace file.
    if (mIfs) {
        if (mIfs->is_open()) {
            mIfs->close();
        }

        delete mIfs;
        mIfs = NULL;
    }

    // If we have any cursors, clean them out.
    //Beware, clear() doesn't destruct classes!
    if (mCursors.size()) {
        for (map<string, tmCursor *>::iterator i = mCursors.begin(); i != mCursors.end(); ++i) {
            cerr << endl << "FREEING CURSOR: " << i->second->CursorId() << endl;
            cerr << *(i->second);
            mCursors.erase(i);
            delete i->second;
        }
    }
}


/** @brief Initialises the internals of a trace file object.
 *
 * When I had two different constructors, I needed to initialise
 * the members in each. Extracted that code to this function.
 * Since then, I've removed all but one of the constructors, but
 * I have not decided yet, that I'll stick with that option.
 */
void tmTraceFile::init() {
    mTraceFileName = "";
    mOriginalTraceFileName = "";
    mDatabaseVersion = "";
    mOracleHome = "";
    mInstanceName = "";
    mSystemName = "";
    mNodeName = "";
    mLineNumber = 0;
    mIfs = NULL;
}


/** @brief Constructor for a tmTraceFile object.
 *
 * @param TraceFileName std::string.
 *
 * Constructs a new tmTraceFile class, knowing the name of the trace file to
 * be used.
 */
tmTraceFile::tmTraceFile(string TraceFileName)
{
    init();
    mTraceFileName = TraceFileName;
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

    // The main parsing loop. What kind of line have we
    // read? Deal with it accordingly.
    while (readTraceLine(&traceLine)) {

        // Start with the shortest substr() that gives
        // a unique string for any "verbs"!
        string chunk = traceLine.substr(0, 7);

        // PARSING IN CURSOR #cursorID
        if (chunk == "PARSING") {
            if (!parsePARSING(traceLine)) {
                return false;
            };
            continue;
        }

        // PARSE ERROR
        if (chunk == "PARSE E") {
            continue;
        }

        // XCTEND (COMMIT/ROLLBACK)
        if (chunk == "XCTEND ") {
            continue;
        }

        // PARSE #cursorID
        if (chunk == "PARSE #") {
            if (!parsePARSE(traceLine)) {
                return false;
            };
            continue;
        }

        // BINDS #cursorID
        if (chunk == "BINDS #") {
            continue;
        }

        // CLOSE #cursorID
        if (chunk == "CLOSE #") {
            // At present, we no longer care about CLOSEing a cursor.
            continue;
        }

        // ERROR #cursorID
        if (chunk == "ERROR #") {
            continue;
        }

        // Need to shorten things if we get this far!
        // EXEC #cursorID
        chunk = chunk.substr(0, 6);
        if (chunk == "EXEC #") {
            continue;
        }

        // We don't need the continue above, to be honest.
        // But if I ever add another check here, I'll probably
        // forget to add one in the check above. It's what I do! :(
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

    string traceLine;
    bool ok = readTraceLine(&traceLine);

    if (!ok) {
        cerr << "parseHeader(): Cannot read first line from "
             << mTraceFileName << endl;
        return false;
    }

    // Have we got a trace file? The first line starts "Trace file ..."
    if (traceLine.length() > 10) {
        string chunk = traceLine.substr(0, 10);
        if (!(chunk == "Trace file")) {
            cerr << mTraceFileName << " is not an Oracle trace file." << endl;
            cerr << "Missing 'Trace file' in header." << endl;
            return false;
        }
    } else {
        cerr << mTraceFileName << " is not an Oracle trace file." << endl;
        cerr << "Missing 'Trace file' in header." << endl;
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
                 << mTraceFileName << endl;
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

    if (mTraceFileName.empty()) {
        return false;
    }

    mIfs = new ifstream(mTraceFileName);

    if (!mIfs->good()) {
        cerr << "Cannot open trace file " << mTraceFileName << endl;
        cleanUp();
        return false;
    }

    // Read in the header stuff, and attempt to validate this file
    // as an Oracle trace file. It could be something else!
    if (!parseHeader()) {
        cleanUp();
        return false;
    };

    // Looks like a valid trace file.
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
    try {
        getline(*mIfs, *aLine);
    } catch (exception e) {
        cerr << "EXCEPTION: readTraceLine(): " << e.what() << "." << endl;
        return false;
    }

    mLineNumber++;
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

    // PARSING IN CURSOR #4572676384 len=229 dep=1 ...
    regex reg("PARSING IN CURSOR\\s(#\\d+)\\slen=(\\d+)\\sdep=(\\d+).*");
    smatch match;

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
    // It's a pity that the trace doesn't show an easy way to find a parent depth
    // zero cursor for all the cursors with depth > zero. That would be nice.
    if (!depth) {
        tmCursor *thisCursor = new tmCursor(cursorID, sqlLength, sqlLine);
        if (!thisCursor) {
            cerr << "parsePARSING(): Cannot allocate a new tmCursor." << endl;
            return false;
        }

        // Extract the SQL Text.
        // For efficiency, pre-allocate the current sql length, plus a bit.
        string sqlText = "";
        string aLine;
        sqlText.reserve(sqlLength + 20);

        while (readTraceLine(&aLine)) {
            if (aLine.substr(0, 11) == "END OF STMT") {
                break;
            }

            if (!sqlText.empty()) {
                    sqlText += string("\n");
            }

            sqlText += aLine;
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
            exists.first->second->SetSQLLineNumber(sqlLine);
            exists.first->second->SetSQLLength(sqlLength);
            exists.first->second->SetSQLParseLine(0);
            delete thisCursor;
        }

        // Then set the SQL Text, regardless.
        exists.first->second->SetSQLText(sqlText);
    }

    // Looks like a good parse.
    return true;

}

/** @brief Parses a "PARSE" line.
 *
 * Parses a line from the trace file. The line is expected
 * to be the PARSE #cursor line.
 *
 * The tmCursor associated with this PARSE is found, and updated to the new
 * source file line number. Only the most recent PASRE is stored for each tmCursor.
 *
 * Returns true if all ok. False otherwise.
 */
bool tmTraceFile::parsePARSE(const string &thisLine) {

    // PARSE #4572676384 ... dep=1 ...
    regex reg("PARSE\\s(#\\d+).*?dep=(\\d+).*");
    smatch match;

    // Extract the cursorID, the length and the depth.
    if (!regex_match(thisLine, match, reg)) {
        cerr << "parsePARSE(): Cannot match regex against PARSE # at line: "
             <<  mLineNumber << "." << endl;
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
            i->second->SetSQLParseLine(mLineNumber);
        } else {
            // Not found. Oh dear!
            cerr << "parsePARSE(): Found PARSE for cursor " << cursorID
                 << " but not found in existing cursor list." << endl;
            return false;
        }
    }

    // Looks like a good parse.
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
        return mCursors.find(cursorID);
}
