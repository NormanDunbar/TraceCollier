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

#ifndef TMTRACEFILE_H
#define TMTRACEFILE_H

/** @file tmtracefile.h
 * @brief Header file for the tmTraceFile object.
 */

#include <string>
#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <exception>

#if defined (USE_REGEX)
    #include <regex>
#endif // defined

#include <sstream>
#include <vector>

using std::string;
using std::map;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::cerr;
using std::cout;
using std::endl;
using std::getline;
using std::exception;
using std::vector;

#if defined (USE_REGEX)
    using std::regex;
    using std::smatch;
#endif // defined

using std::stoul;
using std::numpunct;
using std::locale;
using std::setw;
using std::setfill;

#include "tmcursor.h"
#include "tmoptions.h"

// Some constants used to format the (text) report.
// Maximum of 9,999,999 for a line number.
const int MAXLINENUMBER=10;
const int MAXCURSORWIDTH=11+1;

// Oracle Command codes. We only use COMMAND_PLSQL at the moment.
const int COMMAND_PLSQL = 47;

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
        map<string, tmCursor *> mCursors;    /**< Std::map holding all depth=0 cursors for this trace file. */
        unsigned mLineNumber;                /**< Current line number being parsed. */
        int mExecCount;                      /**< How many EXEC statements have we hit so far? */
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
        void reportHeadings();              /**< Prints HTML headings. */
        bool parseTraceFile();              /**< Parses the trace file body. */
        bool readTraceLine(string *aLine);  /**< Read one line from the trace, update the current line number. */
        map<string, tmCursor *>::iterator findCursor(const string &cursorID);   /**< Finds a cursor id in the cursor list. */
        string mUnprocessedLine;            /**< ParseBINDS() read ahead line. */

        // Parsing stuff.
        bool parsePARSING(const string &thisLine);  /**< Parses a PARSING IN CURSOR line. */
        bool parsePARSE(const string &thisLine);    /**< Parses a PARSE line. */
        bool parseEXEC(const string &thisLine);     /**< Parses an EXEC line. */
        bool parsePARSEERROR(const string &thisLine);    /**< Parses a PARSE line. */
        bool parseXCTEND(const string &thisLine);   /**< Parses a PARSE line. */
        bool parseERROR(const string &thisLine);    /**< Parses a PARSE line. */
        bool parseBINDS(const string &thisLine);    /**< Parses a BINDS line. */
        bool parseBindData(tmBind *thisBind, vector<string>::iterator i);       /**< Parses a bind's data lines. */
        bool parseCLOSE(const string &thisLine);    /**< Parses a CLOSE line. */
        bool parseSTAT(const string &thisLine);    /**< Parses a STAT line. */

        // Data extraction from a vector of bind lines.
        bool extractBindData(const vector<string>::iterator start, const vector<string>::iterator stop, tmCursor *thisCursor, tmBind *thisBind);    /**< Extracts the bind data from a vector. */
        bool extractNumber(vector<string>::const_iterator i, const unsigned equalPos, unsigned &result);  /**< Extracts a numeric value. */
        bool extractHex(vector<string>::const_iterator i, const unsigned equalPos, string &result);  /**< Extracts a hex value. */
        bool extractBindValue(vector<string>::const_iterator i, tmBind *thisBind);  /**< Extracts a string representing a bind's actual value. */
};

// Stolen from http://stackoverflow.com/questions/4728155/how-do-you-set-the-cout-locale-to-insert-commas-as-thousands-separators
// to allow me to automagically insert ',' (or '.' depending on the locale) into big numbers.
// I'll be using this on the report and debug files.
//
// Note.
// I'm not convinced that this requires the parameter or the do_thousands_sep()
// function as without it, I still get commas - which I think are from the locale.
// I might be wrong. It happens. But I'm leaving them out for now in the hope that
// "foreign" locales will get their chosen separator too.
/** @brief A class that is 'imbued' onto the output streams to allow locale
 *         dependant thousands separators to be applied to all numbers
 *         output to that stream.
 */template<typename T> class ThousandsSeparator : public numpunct<T> {
public:
    ThousandsSeparator(T Separator) : m_Separator(Separator) {}

protected:
//    T do_thousands_sep() const  {
//        return m_Separator;
//    }

    virtual string do_grouping() const
    {
        return "\03";  // Ie, every three digits.
    }

private:
    T m_Separator;
};


#endif // TMTRACEFILE_H
