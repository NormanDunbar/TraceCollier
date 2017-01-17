#include "TraceMiner2.h"

tmOptions::tmOptions()
{
    mHelp = true;
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

    mTraceFile = "C:\\Users\\ndunbar\\Downloads\\Oracle-Azure stuff\\TraceMinerTraces\\jdepy1t_ora_48496808.trc";
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




