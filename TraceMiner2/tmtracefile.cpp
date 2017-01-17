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
}


void tmTraceFile::init() {
    mTraceFileName = "";
    mOriginalTraceFileName = "";
    mDatabaseVersion = "";
    mOracleHome = "";
    mInstanceName = "";
    mSystemName = "";
    mNodeName = "";
    mTraceFileDate = "";
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
    cout << "tmTracefile destroyed!" << endl;
}


/** @brief Parses a trace file.
  *
  * If a trace file is known, the Parse() function will parse it and report
  * the various binds etc used in the EXEC lines found. Only 'DEP=0' EXECs are
  * considered. Returns a pointer to the ifstream used to open the file. Or NULL.
  */
bool tmTraceFile::Parse()
{
    // Process a trace file.
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

    bool ok = true;

    string traceLine = readTraceLine(&ok);
    if (!ok) {
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
    mOriginalTraceFileName = traceLine.substr(12);

    while (true) {
        traceLine = readTraceLine(&ok);
        if (!ok)
            break;

        string chunk = traceLine.substr(0, 10);

        if (chunk == "Oracle Dat") {
            mDatabaseVersion = traceLine.substr(17);
            continue;
        }

        if (chunk == "ORACLE HOM") {
            mDatabaseVersion = traceLine.substr(15);
            continue;
        }

        if (chunk == "System nam") {
            mDatabaseVersion = traceLine.substr(14); // Hmm. TAB character here sometimes!
            continue;
        }

        if (chunk == "Node name:") {
            mDatabaseVersion = traceLine.substr(12);
            continue;
        }

        if (chunk == "*** SESSIO") {
            mTraceFileDate = traceLine.substr(28);
            continue;
        }

        if (chunk == "==========") {
            break;
        }

    }

    if (!ok)
        return false;

    return true;
}


/** @brief Opens a trace file.
  *
  * Open a trace file and read in the header details.
  */
ifstream *tmTraceFile::openTraceFile()
{
    if (mTraceFileName.empty()) {
        return NULL;
    }

    ifstream *mIfs = new ifstream(mTraceFileName);

    if (!mIfs->good()) {
        cleanUp();
        return mIfs;
    }

    // Read in the header stuff, and attempt to validate this file
    // as an Oracle trace file. It could be something else!
    if (!parseHeader()) {
        cleanUp();
        return mIfs;
    };

    // Looks like a valid trace file.
    return mIfs;

}


/** @brief Reads a single line from a trace file.
  *
  * Reads a single line from a trace file. Updated the current
  * line number within the trace file. Sets bool parameter if we
  * are still good for more reading.
  */
string tmTraceFile::readTraceLine(bool *ok) {
    string aLine;
    getline(*mIfs, aLine);
    *ok = mIfs->good();
    return aLine;
}
