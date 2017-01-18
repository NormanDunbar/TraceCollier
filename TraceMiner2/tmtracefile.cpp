#include "tmtracefile.h"

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
        tmCursor *me;
        for (map<string, tmCursor *>::iterator iter = mCursors.begin(); iter != mCursors.end(); ++iter) {

            THIS BIT DOESN'T COMPILE.

            me = *iter;
            cerr << "FREE: " << me->CursorId() << endl;
            mCursors.erase(iter);
            free me;
        }
    }
}


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


/** @brief Default constructor.
  *
  * Constructs a new tmTraceFile class without knowing which actual trace file
  * is to be used. Must call SetTraceFile() before attempting to parse anything.
  */
tmTraceFile::tmTraceFile()
{
    init();
}


/** @brief Constructor with known trace file name.
  *
  * Constructs a new tmTraceFile class, knowing the name of the trace file to
  * be used.
  */
tmTraceFile::tmTraceFile(string TraceFileName)
{
    init();
    mTraceFileName = TraceFileName;
}


/** @brief Destructor.
  *
  * Nothing to see here. Cleans up as required.
  */
tmTraceFile::~tmTraceFile()
{
    // Destructor.
    // If still open, close the trace file.
    cleanUp();
}


/** @brief Parses a trace file.
  *
  * If a trace file is known, the Parse() function will parse it and report
  * the various binds etc used in the EXEC lines found. Only 'DEP=0' EXECs are
  * considered. Returns a boolean indicating success or failure.
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
            parsePARSING(traceLine);
        }

        // PARSE ERROR
        if (chunk == "PARSE E") {
        }

        // XCTEND (COMMIT/ROLLBACK)
        if (chunk == "XCTEND ") {
        }

        // PARSE #cursorID
        if (chunk == "PARSE #") {
        }

        // BINDS #cursorID
        if (chunk == "BINDS #") {
        }

        // CLOSE #cursorID
        if (chunk == "CLOSE #") {
        }

        // ERROR #cursorID
        if (chunk == "ERROR #") {
        }

        // Need to shorten things if we get this far!
        // EXEC #cursorID
        chunk = chunk.substr(0, 6);
        if (chunk == "EXEC #") {
        }

    }

    return true;
}


/** @brief Parses a trace file header.
  *
  * Validates that this looks to be an Oracle trace file, then
  * reads in a few details from the header.
  *
  * On exit from here, the next read will read the first line
  * after the first "==============" which should normally be
  * a PARSING IN CURSOR line. Normally!
  */
bool tmTraceFile::parseHeader() {

    string traceLine;
    bool ok = readTraceLine(&traceLine);

    if (!ok) {
        cerr << "parseHeader(): Cannot read first line from " << mTraceFileName << endl;
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
            cerr << "parseHeader(): Cannot read header line(s) from " << mTraceFileName << endl;
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
  * Open a trace file and read in the header details.
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
  * Reads a single line from a trace file. Updates the current
  * line number within the trace file. Returns bool parameter if we
  * are still good for more reading.
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


/** @brief Parses out a PARSING IN CURSOR line.
  *
  * Parses a line form the trace file. The line is expected
  * to be the PARSING IN CURSOR line. We are only interested in
  * cursors at depth = 1.
  * Creates a tmCursor object and appends it to the map<string, tmCursor *>
  * that we use to hold these things.
  * Returns true if all ok.
  */
bool tmTraceFile::parsePARSING(const string &thisLine) {

    // PARSING IN CURSOR #4572676384 len=229 dep=1 ...
    regex reg("(#\\d+)\\slen=(\\d+)\\sdep=(\\d+)");
    smatch match;

    // Extract the cursorID, the length and the depth.
    if (!regex_search(thisLine, match, reg)) {
        cerr << "parsePARSING(): Cannot match regex against PARSING IN CURSOR at line: " <<  mLineNumber << "." << endl;
        return false;
    }

    // Found it!
    // Extract the goodies. We must have three matches.
    string cursorID = match[1];
    string sqlLength = match[2];
    string depth = match[3];

    if (depth == "0") {
        tmCursor *thisCursor = new tmCursor(cursorID, stoul(sqlLength,NULL,10), mLineNumber);

        // Stash this one.
        mCursors.insert(pair<string, tmCursor *>(cursorID, thisCursor));

        cerr << "CursorID = " << cursorID << endl;
        cerr << "sqlLength = " << sqlLength << endl;
        cerr << "depth = " << depth << endl;

        cerr << "MAP contains " << mCursors.size() << endl << endl;
    }

    // Looks like a good parse.
    return true;

}
