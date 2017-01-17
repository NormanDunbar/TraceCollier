#include "TraceMiner2.h"

tmOptions::tmOptions()
{
    mHelp = false;
    mVerbose = false;
    mHtml = false;
    mTraceFile = "";
    mReportFile = "";
    mDebugFile = "";
}

tmOptions::~tmOptions()
{
    // Nothing to do here!
}


bool tmOptions::ParseArgs(int argc, char *argv[]) {

    bool invalidArgs = false;
    bool gotTraceAlready = false;

    if (argc < 2) {
        // Insufficient args.
        cerr << "TraceMiner2: no arguments supplied." << endl;
        return false;
    }

    for (int arg = 1; arg < argc; arg++) {
        // Convert args to lower case and to strings.
        // Not really safe with Unicode though!
        string thisArg = string(argv[arg]);
        for (int c = 0; c < thisArg.length(); c++) {
            thisArg[c] = tolower(thisArg[c]);
        }

        // Try verbose first ...
        if ((thisArg == "--verbose") ||
        (thisArg == "-v")) {
            mVerbose = true;
            continue;
        }

        // Ok, try HTML instead ...
        if ((thisArg == "--html") ||
        (thisArg == "-m")) {
            mHtml = true;
            continue;
        }

        // Nope? Try help then ...
        if ((thisArg == "--help") ||
        (thisArg == "-h")        ||
        (thisArg == "-?")) {
            mHelp = true;
            continue;
        }

        // Either a filename or an error.
        // Try for an error ...
        if (thisArg[0] == '-') {
            cerr << "TraceMiner2: Invalid argument '";
            cerr << string(argv[arg]) << "'." << endl;
            invalidArgs = true;
            continue;
        }

        // Nope. Must (!) be a filename.
        // Do not lowercase it as we are probably on Unix!
        // And we only want a single trace file.
        if (!gotTraceAlready) {
            mTraceFile = string(argv[arg]);
            gotTraceAlready = true;
        } else {
            // Too many tracefiles.
            cerr << "TraceMiner2: too many trace files. (" << argv[arg] << ")." << endl;
            invalidArgs = true;
        }

    }

    // We need at least a trace file.
    if (mTraceFile.empty()) {
        cerr << "TraceMiner2: no trace file supplied." << endl;
        invalidArgs = true;
    }

    // Did we barf?
    if (invalidArgs) {
        usage();
        return false;
    }

    // Dis we just want help?
    if (mHelp) {
        usage();
        return true;
    }

    // Set up the other files now.
    // Assumes mTraceFile is correct.

    mReportFile = replaceFileExtension(mTraceFile, mReportExtension);
    mDebugFile = replaceFileExtension(mTraceFile, mDebugExtension);

    return true;
}


void tmOptions::usage() {

    cerr << endl << "USAGE:" << endl << endl;
    cerr << "TraceMiner2 [options] trace_file" << endl << endl;
    cerr << "'trace_file' is the Oracle trace file name. It should have binds turned on." << endl << endl;

    cerr << "OPTIONS:" << endl << endl;
    cerr << "'-v' or '--verbose' Turn on verbose mode." << endl;
    cerr << "Lots of text is written to the debugfile." << endl << endl;

    cerr << "'-m' or '--html' Turn on HTML mode. The report file will be in HTML format." << endl;
    cerr << "The default is for the report to be in plain text format." << endl << endl;

    cerr << "'-?'. '-h' or '--help' Displays this help, and exits." << endl << endl;

    cerr << "OUTPUT FILES:" << endl << endl;
    cerr << "'output_file' is where the information you want will be written." << endl;
    cerr << "The file is written to the same location as the trace file." << endl;
    cerr << "and has the file extension '" << mReportExtension << "'." << endl << endl;

    cerr << "'debug_file' is where very verbose information will be written." << endl;
    cerr << "The file is written to the same location as the trace file." << endl;
    cerr << "and has the file extension '" << mDebugExtension << "'." << endl << endl;
}




