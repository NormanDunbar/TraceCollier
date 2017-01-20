#ifndef TMBIND_H
#define TMBIND_H

#include <string>
#include <iostream>


using std::string;
using std::endl;
using std::cerr;
using std::cout;
using std::endl;


/** @brief A class representing a bind variable in a SQL statement.
 */
class tmBind
{
    public:
        tmBind(unsigned id, string name);
        ~tmBind();
        friend ostream &operator<<(ostream &out, tmBind &bind);

        // Getters.
        unsigned BindId() { return mBindId; }                   /**< Returns the bind number. */
        unsigned BindLineNumber() { return mBindLineNumber; }   /**< Returns the linenumber the bind was last seen at. */
        string BindValue() { return mBindValue; }               /**< Returns the most recent value for this bind. */
        string BindName() { return mBindName; }                 /**< Returns the bind variable name as used in the SQL. */

        // Setters.
        void SetBindId(unsigned val) { mBindId = val; }         /**< Sets a bind number. */
        void SetBindValue(string val) { mBindValue = val; }     /**< Sets a new bind value. */
        void SetBindLineNumber(unsigned val) { mBindLineNumber = val; } /**< Sets a new bind line number. */
        void SetBindName(string val) { mBindName = val; }       /**< Sets the bind variable name. */

    protected:

    private:
        unsigned mBindId;               /**< The bind number within the SQL statement. */
        unsigned mBindLineNumber;       /**< The line number the bind details were found at. */
        string mBindValue;              /**< The current EXEC statement's value for this bind. */
        string mBindName;               /**< The name of the bind variable in the SQL statement. */
};

#endif // TMBIND_H
