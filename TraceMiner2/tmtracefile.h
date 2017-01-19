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

        // Getters.
        string TraceFileName() { return mTraceFileName; }
        unsigned lineNumber() { return mLineNumber; }
        string DatabaseVersion() { return mDatabaseVersion; };
        string OriginalTraceFileName() { return mOriginalTraceFileName; };
        string OracleHome() { return mOracleHome; };
        string InstanceName() { return mInstanceName; };
        string SystemName() { return mSystemName; };
        string NodeName() { return mNodeName; };

        // There are no setters.


        bool parseTraceFile();
        bool openTraceFile();

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
        map<string, tmCursor *>::iterator findCursor(const string &cursorID);

        // Parsing stuff.
        bool parsePARSING(const string &thisLine);
        bool parsePARSE(const string &thisLine);

};

#endif // TMTRACEFILE_H
