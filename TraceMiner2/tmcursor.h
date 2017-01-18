#ifndef TMCURSOR_H
#define TMCURSOR_H

#include <string>
#include <iostream>

using std::string;
using std::cout;
using std::endl;
using std::pair;


class tmCursor
{
    public:
        tmCursor(string id, unsigned sqlSize, unsigned sqlLine);
        ~tmCursor();

        string CursorId() { return mCursorId; }
        unsigned SQLLineNumber() { return mSQLLineNumber; }
        unsigned SQLSize() { return mSQLSize; }
        string SQLText() { return mSQLText; }
        void SetSQLText(string val) { mSQLText = val; }
        unsigned SQLParseLine() { return mSQLParseLine; }
        void SetSQLParseLine(unsigned val) { mSQLParseLine = val; }

    protected:

    private:
        string mCursorId;
        unsigned mSQLLineNumber;
        unsigned mSQLSize;
        string mSQLText;
        unsigned mSQLParseLine;
};

#endif // TMCURSOR_H
