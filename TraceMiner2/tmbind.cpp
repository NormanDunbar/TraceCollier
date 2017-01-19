#include "tmbind.h"

tmBind::tmBind(unsigned id, string name) {

    mBindId = id;
    mBindLineNumber = 0;
    mBindValue = "";
    mBindName = name;
}

tmBind::~tmBind()
{
    //dtor
}


/** @brief Allows a tmBind to be streamed to an ostream.
  *
  * This function/operator allows a tmBind to stream itself
  * out to an ostream.
  */
ostream &operator<<(ostream &out, tmBind &bind) {
    out << "BindID: " << bind.BindId() << endl;
    out << "Bind Line Number: " << bind.BindLineNumber() << endl;
    out << "Bind Name: " << bind.BindName() << endl;
    out << "Bind Value: " << bind.BindValue() << endl << endl;
    return out;
}

