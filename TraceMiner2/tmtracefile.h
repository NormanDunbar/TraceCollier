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

#if defined (USE_REGEX)
    #include <regex>
#endif // defined

#include <sstream>

using std::string;
using std::map;
using std::ifstream;
using std::ofstream;
using std::cerr;
using std::cout;
using std::endl;
using std::getline;
using std::exception;

#if defined (USE_REGEX)
    using std::regex;
    using std::smatch;
#endif // defined

using std::stoul;
using std::numpunct;
using std::locale;


#include "tmcursor.h"
#include "tmoptions.h"

// Some constants used to format the (text) report.
const int MAXLINENUMBER=7;

/** @brief A class representing an Oracle trace file.
 */
class tmTraceFile
{
    public:
        tmTraceFile(tmOptions *options);
        ~tmTraceFile();

        // Getters.
        unsigned lineNumber() { return mLineNumber; }               /**< Returns the current line number being parsed. */
        string databaseVersion() { return mDatabaseVersion; };      /**< Returns the Oracle version when the trace was created. */
        string originalTraceFileName() { return mOriginalTraceFileName; };  /**< Returns the trace file name as created by the database. */
        string oracleHome() { return mOracleHome; };        /**< Returns the oracle home setting when the trace was created. */
        string instanceName() { return mInstanceName; };    /**< Returns the instance name the trace was created on. */
        string systemName() { return mSystemName; };        /**< Returns the System (aka OS) name, Windows, AIX etc. */
        string nodeName() { return mNodeName; };            /**< returns the database server name. */

        // Setters.
        void setOptions(tmOptions *opt) { mOptions = opt; }   /**< Sets the options pointer. */

        // Other useful stuff.
        bool parse();               /**< Parses the trace file. */

    protected:

    private:
        map<string, tmCursor *> mCursors;   /**< Std::map holding all depth=0 cursors for this trace file. */
        unsigned mLineNumber;               /**< Current line number being parsed. */
        tmOptions *mOptions;                 /**< Pointer to (parsed) command line options. */

        // Stuff from the trace file header.
        string mDatabaseVersion;            /**< File header information - Oracle version. */
        string mOriginalTraceFileName;      /**< File header information - trace file name as created. */
        string mOracleHome;                 /**< File header information - Oracle Home location. */
        string mInstanceName;               /**< File header information - Instance name. */
        string mSystemName;                 /**< File header information - OS Name. */
        string mNodeName;                   /**< File header information - Database server name. */
        ifstream *mIfs;                     /**< Std::ifstream used to read the trace file. */
        ofstream *mOfs;                     /**< Std::ofstream used to write the report file. */
        ofstream *mDbg;                     /**< Std::ofstream used to write the debug file. */

        // Internal stuff.
        void cleanUp();                     /**< Cleans up on destruction etc. */
        bool parseHeader();                 /**< Parse the trace file header details. */
        void init();                        /**< Initialise a tmCursor object. */
        bool openTraceFile();               /**< Opens the trace file and parses the headings. */
        bool openDebugFile();               /**< Opens the debug file. */
        bool openReportFile();              /**< Opens the debug file. */
        bool parseTraceFile();              /**< Parses the trace file body. */
        bool readTraceLine(string *aLine);  /**< Read one line from the trace, update the current line number. */
        map<string, tmCursor *>::iterator findCursor(const string &cursorID);   /**< Finds a cursor id in the cursor list. */

        // Parsing stuff.
        bool parsePARSING(const string &thisLine);  /**< Parses a PARSING IN CURSOR line. */
        bool parsePARSE(const string &thisLine);    /**< Parses a PARSE line. */
        bool parseEXEC(const string &thisLine);    /**< Parses an EXEC line. */
        bool parseBINDS(const string &thisLine);    /**< Parses a BINDS line. */
        bool parsePARSEERROR(const string &thisLine);    /**< Parses a PARSE line. */
        bool parseXCTEND(const string &thisLine);    /**< Parses a PARSE line. */
        bool parseERROR(const string &thisLine);    /**< Parses a PARSE line. */

};

// Stolen from http://stackoverflow.com/questions/4728155/how-do-you-set-the-cout-locale-to-insert-commas-as-thousands-separators
// to allow me to automagically insert ',' (or '.' depending on the locale) into big numbers.
// I'll be using this on the report and debug files - if I can!
template<typename T> class ThousandsSeparator : public numpunct<T> {
public:
    ThousandsSeparator(T Separator) : m_Separator(Separator) {}

protected:
    T do_thousands_sep() const  {
        return m_Separator;
    }

    virtual string do_grouping() const
    {
        return "\03";  // Ie, every three digits.
    }

private:
    T m_Separator;
};


#endif // TMTRACEFILE_H
