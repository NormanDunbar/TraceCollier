#ifndef TMOPTIONS_H
#define TMOPTIONS_H

#include <string>
#include <iostream>

#include "utilities.h"

using std::string;
using std::cerr;
using std::endl;

class tmOptions
{
    public:
        tmOptions();
        virtual ~tmOptions();

        // Getters.
        bool Verbose() { return mVerbose; }
        bool Html() { return mHtml; }
        bool Help() { return mHelp; }

        string traceFile() { return mTraceFile; }
        string reportFile() { return mReportFile; }
        string debugFile() { return mDebugFile; }

        string htmlExtension() { return mHtmlExtension; }
        string reportExtension() { return mReportExtension; }
        string debugExtension() { return mDebugExtension; }

        // There are no Setters.

        void usage();
        bool ParseArgs(int argc, char *argv[]);

    protected:

    private:
        bool mVerbose;
        bool mHtml;
        bool mHelp;
        string mTraceFile;
        string mReportFile;
        string mDebugFile;

        string mReportExtension = "log";
        string mHtmlExtension = "html";
        string mDebugExtension = "dbg";
};

#endif // TMOPTIONS_H
