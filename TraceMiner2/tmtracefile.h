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
        bool parseTraceFile();
        bool openTraceFile();
        string DatabaseVersion() { return mDatabaseVersion; };
        string OriginalTraceFileName() { return mOriginalTraceFileName; };
        string OracleHome() { return mOracleHome; };
        string InstanceName() { return mInstanceName; };
        string SystemName() { return mSystemName; };
        string NodeName() { return mNodeName; };

    protected:

    private:
        string mTraceFileName;
        map<string, tmCursor *> mCursors;

        // Stuff from the trace file header.
        string mDatabaseVersion;
        string mOriginalTraceFileName;
        string mOracleHome;
        string mInstanceName;
        string mSystemName;
        string mNodeName;
        ifstream *mIfs;

        // Internal stuff.
        void cleanUp();
        bool parseHeader();
        void init();
        bool readTraceLine(string *aLine);
        // Where the hell are we?
        unsigned mLineNumber;

};

#endif // TMTRACEFILE_H
