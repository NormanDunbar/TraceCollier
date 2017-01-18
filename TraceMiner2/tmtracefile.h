#ifndef TMTRACEFILE_H
#define TMTRACEFILE_H

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <exception>
#include <regex>

using std::string;
using std::map;
using std::ifstream;
using std::cerr;
using std::cout;
using std::endl;
using std::getline;
using std::exception;
using std::regex;
using std::smatch;
using std::stoul;

#include "tmcursor.h"

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
        unsigned mLineNumber;

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
        bool parsePARSING(const string &thisLine);

};

#endif // TMTRACEFILE_H
