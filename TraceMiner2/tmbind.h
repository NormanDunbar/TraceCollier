#ifndef TMBIND_H
#define TMBIND_H

#include "TraceMiner2.h"


class tmBind
{
    public:
        tmBind();
        tmBind(unsigned BindID);
        ~tmBind();

        unsigned BindId() { return mBindId; }
        void SetBindId(unsigned val) { mBindId = val; }
        unsigned BindLineNumber() { return mBindLineNumber; }
        void SetBindLineNumber(unsigned val) { mBindLineNumber = val; }
        string BindValue() { return mBindValue; }
        void SetBindValue(string val) { mBindValue = val; }

    protected:

    private:
        unsigned mBindId;
        unsigned mBindLineNumber;
        string mBindValue;
};

#endif // TMBIND_H
