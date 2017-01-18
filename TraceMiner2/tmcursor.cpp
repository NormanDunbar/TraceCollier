#include "tmcursor.h"

tmCursor::tmCursor(string id, unsigned sqlSize, unsigned sqlLine)
{
    mCursorId = id;
    mSQLLineNumber = sqlLine;
    unsigned mSQLSize = sqlSize;
    mSQLText = "";
    mSQLParseLine = 0;

}

tmCursor::~tmCursor()
{
    cout << "Destroying mCursorID: " << mCursorId << endl;
}
