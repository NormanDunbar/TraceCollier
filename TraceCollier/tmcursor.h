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

#ifndef TMCURSOR_H
#define TMCURSOR_H

/** @file tmcursor.h
 * @brief Header file for the tmCursor object.
 */

#include <string>
#include <iostream>
#include <map>
#include <regex>

using std::string;
using std::cout;
using std::endl;
using std::pair;
using std::map;
using std::regex;
using std::smatch;
using std::ostream;

#include "tmbind.h"

/** @brief A class representing a cursor variable in an Oracle trace file.
 */
class tmCursor
{
    public:
        tmCursor(string id, unsigned sqlSize, unsigned sqlLine);
        ~tmCursor();
        friend ostream &operator<<(ostream &out, const tmCursor &cursor);

        // Getters.
        string cursorId() { return mCursorId; }                 /**< Returns the cursor id, including  the # prefix. */
        unsigned sqlLineNumber() { return mSQLLineNumber; }     /**< Returns the line number where the SQL can be found. */
        unsigned sqlLength() { return mSQLSize; }               /**< Returns the size of the SQL statement. */
        string sqlText() { return mSQLText; }                   /**< Returns the SQL statement. */
        unsigned sqlParseLine() { return mSQLParseLine; }       /**< Returns the most recent parse line number for this statement. */
        unsigned bindCount() { return mBindCount; }             /**< Returns the number of binds for this statement. */
        unsigned commandType() { return mCommandType; }         /**< Returns the command type for this statement. */
        unsigned bindsLine() { return mBindsLine; }             /**< Returns the last "BINDS #cursor" line number for this statement. */
        map<unsigned, tmBind *> *binds() { return &mBinds; }    /**< Returns a pointer to the binds for this statement. */
        bool isClosed() { return mClosed; }                     /**< Returns whether or not the cursor is closed. */
        bool isReturning() { return mReturning; }               /**< Returns whether or not the cursor has a RETURNING clause. */
        string getLocal() { return mLocal; }                    /**< Returns the local date/time of the cursor */
        unsigned execLine() { return mExecLine; }                    /**< Returns the last EXEC line for the cursor. */

        // Setters.
        void setSQLText(string val);                            /**< Changes the SQL statement for this cursor. */
        void setSQLLength(unsigned val) { mSQLSize = val; }     /**< Changes the size of the SQL text. */
        void setSQLParseLine(unsigned val) { mSQLParseLine = val; }     /**< Changes the parse line number. */
        void setSQLLineNumber(unsigned val) { mSQLLineNumber = val; }   /**< Changes the SQL line number. */
        void setBindCount(unsigned val) { mBindCount = val; }           /**< Changes the number of binds for the statement. */
        void setBindsLine(unsigned val) { mBindsLine = val; }           /**< Changes the "BINDS #cursor" line number for the statement. */
        void setCommandType(unsigned val) { mCommandType = val; }       /**< Changes the command type for the statement. */
        void setClosed(bool val) { mClosed = val; }                     /**< Changes the closed state of the cursor. */
        void setReturning(bool val) { mReturning = val; }               /**< Changes the RETURNING state of the cursor. */
        void setLocal(string val) { mLocal = val; }                     /**< Changes the local date.time of the cursor. */
        void setExec(unsigned val) { mExecLine = val; }                /**< Changes the EXEC line of the cursor. */

    protected:

    private:
        string mCursorId;                   /**< Cursor ID including the # prefix. */
        unsigned mSQLLineNumber;            /**< Line in the trace where the SQL can be found. */
        unsigned mSQLSize;                  /**< What Oracle reports the size of the SQL statement to be. */
        string mSQLText;                    /**< The actual SQL text, extracted from the trace file. */
        unsigned mSQLParseLine;             /**< Line in the trace file where this statement was most recently parsed. */
        unsigned mBindCount;                /**< How many binds are there in this statement? */
        unsigned mCommandType;              /**< What command is executing in this statement? */
        unsigned mBindsLine;                /**< The line where we found the most recent "BINDS #cursor" for this cursor */
        map<unsigned, tmBind *> mBinds;     /**< A std::map of all found binds for this statement. Indexed by bind position. */
        bool mClosed;                       /**< Has this cursor been closed recently? */
        bool mReturning;                    /**< Does this cursor have a RETURNING clause? */
        unsigned mStopScanningHere;         /**< Where to stop looking for bind variables in the string. */
        string mLocal;                       /**< Local date/time for this exec */
        unsigned mExecLine;                 /**< Line number of previous EXEC - for parseERROR() */

        bool buildBindMap(const string &sql);
        void cleanUp();
};

#endif // TMCURSOR_H
