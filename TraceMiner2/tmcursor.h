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

        // Setters.
        void setSQLText(string val);                            /**< Changes the SQL statement for this cursor. */
        void setSQLLength(unsigned val) { mSQLSize = val; }     /**< Changes the size of the SQL text. */
        void setSQLParseLine(unsigned val) { mSQLParseLine = val; }     /**< Changes the parse line number. */
        void setSQLLineNumber(unsigned val) { mSQLLineNumber = val; }   /**< Changes the SQL line number. */
        void setBindCount(unsigned val) { mBindCount = val; }   /**< Changes the number of binds for the statement. */

    protected:

    private:
        string mCursorId;                   /**< Cursor ID including the # prefix. */
        unsigned mSQLLineNumber;            /**< Line in the trace where the SQL can be found. */
        unsigned mSQLSize;                  /**< What Oracle reports the size of the SQL statement to be. */
        string mSQLText;                    /**< The actual SQL text, extracted from the trace file. */
        unsigned mSQLParseLine;             /**< Line in the trace file where this statement was most recently parsed. */
        unsigned mBindCount;                /**< How many binds are there in this statement? */
        map<unsigned, tmBind *> mBinds;     /**< A std::map of all found binds for this statement. Indexed by bind position. */

        bool buildBindMap(const string &sql);
        void cleanUp();
};

#endif // TMCURSOR_H
