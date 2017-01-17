#ifndef TMTRACEFILE_H
#define TMTRACEFILE_H

#include "TraceMiner2.h"


class tmCursor;
class ifstream;

class tmTraceFile
{
    public:
        tmTraceFile();
        tmTraceFile(string TraceFileName);
        ~tmTraceFile();

        string TraceFileName() { return mTraceFileName; }
        void SetTraceFileName(string val) { mTraceFileName = val; }
        unsigned lineNumber() { return mLineNumber; }
        bool Parse();

    protected:

    private:
        string mTraceFileName;
        map<string, tmCursor> mCursors;

        // Stuff from the trace file header.
        string mDatabaseVersion;
        string mOriginalTraceFileName;
        string mOracleHome;
        string mInstanceName;
        string mSystemName;
        string mNodeName;
        string mTraceFileDate;
        ifstream *mIfs;

        // Internal stuff.
        void cleanUp();
        bool parseHeader();
        void init();
        ifstream *openTraceFile();
        string readTraceLine(bool *ok);
        // Where the hell are we?
        unsigned mLineNumber;

};

#endif // TMTRACEFILE_H
