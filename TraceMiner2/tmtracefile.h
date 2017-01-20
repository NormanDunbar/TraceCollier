#ifndef TMTRACEFILE_H
#define TMTRACEFILE_H

/** @file tmtracefile.h
 * @brief Header file for the tmTraceFile object.
 */

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

/** @brief A class representing an Oracle trace file.
 */
class tmTraceFile
{
    public:
        tmTraceFile();
        tmTraceFile(string TraceFileName);
        ~tmTraceFile();

        // Getters.
        string TraceFileName() { return mTraceFileName; }           /**< Returns the trace file name. */
        unsigned lineNumber() { return mLineNumber; }               /**< Returns the current line number being parsed. */
        string DatabaseVersion() { return mDatabaseVersion; };      /**< Returns the Oracle version when the trace was created. */
        string OriginalTraceFileName() { return mOriginalTraceFileName; };  /**< Returns the trace file name as created by the database. */
        string OracleHome() { return mOracleHome; };        /**< Returns the oracle home setting when the trace was created. */
        string InstanceName() { return mInstanceName; };    /**< Returns the instance name the trace was created on. */
        string SystemName() { return mSystemName; };        /**< Returns the System (aka OS) name, Windows, AIX etc. */
        string NodeName() { return mNodeName; };            /**< returns the database server name. */

        // There are no setters.

        bool parseTraceFile();      /**< Parses the trace file body. */
        bool openTraceFile();       /**< Opens the trace file and parses the headings. */

    protected:

    private:
        string mTraceFileName;              /**< Name of the trace file, passed as a command line parameter. */
        map<string, tmCursor *> mCursors;   /**< Std::map holding all depth=0 cursors for this trace file. */
        unsigned mLineNumber;               /**< Current line number being parsed. */

        // Stuff from the trace file header.
        string mDatabaseVersion;            /**< File header information - Oracle version. */
        string mOriginalTraceFileName;      /**< File header information - trace file name as created. */
        string mOracleHome;                 /**< File header information - Oracle Home location. */
        string mInstanceName;               /**< File header information - Instance name. */
        string mSystemName;                 /**< File header information - OS Name. */
        string mNodeName;                   /**< File header information - Database server name. */
        ifstream *mIfs;                     /**< Std::ifstream used to read the trace file.. */

        // Internal stuff.
        void cleanUp();                     /**< Cleans up on destruction etc. */
        bool parseHeader();                 /**< Parse the trace file header details. */
        void init();                        /**< Initialise a tmCursor object. */
        bool readTraceLine(string *aLine);  /**< Read one line from the trace, update the current line number. */
        map<string, tmCursor *>::iterator findCursor(const string &cursorID);   /**< Finds a cursor id in the cursor list. */

        // Parsing stuff.
        bool parsePARSING(const string &thisLine);  /**< Parses a PARSING IN CURSOR line. */
        bool parsePARSE(const string &thisLine);    /**< Parses a PARSE line. */

};

#endif // TMTRACEFILE_H
