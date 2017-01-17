#ifndef TMCURSOR_H
#define TMCURSOR_H

#include "TraceMiner2.h"

class tmCursor
{
    public:
        tmCursor();
        ~tmCursor();

        string CursorId() { return mCursorId; }
        void SetCursorId(string val) { mCursorId = val; }
        unsigned SQLLineNumber() { return mSQLLineNumber; }
        void SetSQLLineNumber(unsigned val) { mSQLLineNumber = val; }
        unsigned SQLSize() { return mSQLSize; }
        void SetSQLSize(unsigned val) { mSQLSize = val; }
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
