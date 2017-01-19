#ifndef TMBIND_H
#define TMBIND_H

#include <string>
#include <iostream>


using std::string;
using std::endl;
using std::cerr;
using std::cout;
using std::endl;


class tmBind
{
    public:
        tmBind(unsigned id, string name);
        ~tmBind();
        friend ostream &operator<<(ostream &out, tmBind &bind);

        // Getters.
        unsigned BindId() { return mBindId; }
        unsigned BindLineNumber() { return mBindLineNumber; }
        string BindValue() { return mBindValue; }
        string BindName() { return mBindName; }

        // Setters.
        void SetBindId(unsigned val) { mBindId = val; }
        void SetBindValue(string val) { mBindValue = val; }
        void SetBindLineNumber(unsigned val) { mBindLineNumber = val; }
        void SetBindName(string val) { mBindName = val; }

    protected:

    private:
        unsigned mBindId;
        unsigned mBindLineNumber;
        string mBindValue;
        string mBindName;
};

#endif // TMBIND_H
