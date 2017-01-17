#ifndef TMTRACEFILE_H
#define TMTRACEFILE_H

#include "TraceMiner2.h"


class tmCursor;

class tmTraceFile
{
    public:
        tmTraceFile();
        tmTraceFile(string TraceFileName);
        ~tmTraceFile();

        string TraceFileName() { return mTraceFileName; }
        void SetTraceFileName(string val) { mTraceFileName = val; }
        void Parse();

    protected:

    private:
        string mTraceFileName;
        map<string, tmCursor> mCursors;

        // Stuff from the trace file header.
        string mDatabaseVersion;
        string mOracleHome;
        string mInstanceName;
        string mSystemName;
        string mNodeName;

};

#endif // TMTRACEFILE_H
