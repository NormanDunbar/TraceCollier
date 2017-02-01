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

#include "tmoptions.h"

/** @file tmoptions.cpp
 * @brief Implementation file for the tmOptions object.
 */

/** @brief Constructor for a tmOptions object.
 */
tmOptions::tmOptions()
{
    mHelp = false;
    mVerbose = false;
    mHtml = false;
    mTraceFile = "";
    mReportFile = "";
    mDebugFile = "";
    mCssFileName = "";
}

/** @brief Destructor for a tmOptions object.
 */
tmOptions::~tmOptions()
{
    // Nothing to do here!
}


/** @brief Parses the command line arguments.
 *
 * @param argc int. Number of command line arguments supplied. Plus 1 for the program name.
 * @param argv[] char*. Array of command line arguments.
 * @return bool.
 *
 * Checks for various flags, and makes sure we have a single trace file name.
 * Returns true if all ok, displays usage() and returns false otherwise.
 */
bool tmOptions::parseArgs(int argc, char *argv[]) {

    bool invalidArgs = false;
    bool gotTraceAlready = false;

    if (argc < 2) {
        // Insufficient args.
        cerr << "TraceMiner2: No arguments supplied." << endl;
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
            (thisArg == "-h")     ||
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
            cerr << "TraceMiner2: Too many trace files. (" << argv[arg] << ")." << endl;
            invalidArgs = true;
        }

    }

    // Did we just want help?
    // Do it and bale out.
    if (mHelp) {
        usage();
        return true;
    }

    // We need at least a trace file.
    if (mTraceFile.empty()) {
        cerr << "TraceMiner2: No trace file supplied." << endl;
        invalidArgs = true;
    }

    // Did we barf?
    if (invalidArgs) {
        usage();
        return false;
    }

    // Set up the other files now.
    // Assumes mTraceFile is correct.

    if (mHtml) {
        mReportFile = replaceFileExtension(mTraceFile, mHtmlExtension);
        mCssFileName = filePath(mTraceFile) + directorySeparator + "TraceMiner2.css";
    } else {
        mReportFile = replaceFileExtension(mTraceFile, mReportExtension);
    }
    mDebugFile = replaceFileExtension(mTraceFile, mDebugExtension);

    return true;
}

/** @brief Display brief TraceMiner2 usage details.
 *
 * Displays brief details of how to use TraceMiner2.
 */
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




