//==============================================//
// Copyright (c) Norman Dunbar, 2016.
//==============================================//
// If you wish to use this code, be my guest.
// Just leave my copyright intact, and add a
// wee detail of what it was you changed.
// That's it.
//==============================================//

#include "TraceMiner2.h"
#include "utilities.h"


// Version number.
const float version = 0.01;

// Various flags set according to the passed parameters.
tmOptions options;

int main(int argc, char *argv[])
{
    // Parse command line args and bale if problems detected.
    bool allOk = options.ParseArgs(argc, argv);
    if (!allOk)
        return 1;

    if (options.Help()) {
        options.usage();
        return 0;
    }

    cout << "Tracefile   " << options.traceFile() << endl;
    cout << "ReportFile " << options.reportFile() << endl;
    cout << "DebugFile " << options.debugFile() << endl;

}
