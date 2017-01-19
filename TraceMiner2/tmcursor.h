#ifndef TMCURSOR_H
#define TMCURSOR_H

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

#include "tmbind.h"

class tmCursor
{
    public:
        tmCursor(string id, unsigned sqlSize, unsigned sqlLine);
        ~tmCursor();
        friend ostream &operator<<(ostream &out, tmCursor &cursor);

        // Getters.
        string CursorId() { return mCursorId; }
        unsigned SQLLineNumber() { return mSQLLineNumber; }
        unsigned SQLLength() { return mSQLSize; }
        string SQLText() { return mSQLText; }
        unsigned SQLParseLine() { return mSQLParseLine; }
        unsigned BindCount() { return mBindCount; }

        // Setters.
        void SetSQLText(string val);
        void SetSQLLength(unsigned val) { mSQLSize = val; }
        void SetSQLParseLine(unsigned val) { mSQLParseLine = val; }
        void SetSQLLineNumber(unsigned val) { mSQLLineNumber = val; }
        void SetBindCount(unsigned val) { mBindCount = val; }

    protected:

    private:
        string mCursorId;
        unsigned mSQLLineNumber;
        unsigned mSQLSize;
        string mSQLText;
        unsigned mSQLParseLine;
        unsigned mBindCount;
        map<unsigned, tmBind *> mBinds;

        bool buildBindMap(const string &sql);
        void cleanUp();
};

#endif // TMCURSOR_H
