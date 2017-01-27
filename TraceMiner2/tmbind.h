#ifndef TMBIND_H
#define TMBIND_H

/** @file tmbind.h
 * @brief Header file for the tmBind object.
 */


#include <string>
#include <iostream>


using std::string;
using std::endl;
using std::cerr;
using std::cout;
using std::endl;
using std::ostream;


/** @brief A class representing a bind variable in a SQL statement.
 */
class tmBind
{
    public:
        tmBind(unsigned id, string name);
        ~tmBind();
        friend ostream &operator<<(ostream &out, const tmBind &bind);

        // Getters.
        unsigned bindId() { return mBindId; }                   /**< Returns the bind number. */
        unsigned bindLineNumber() { return mBindLineNumber; }   /**< Returns the linenumber the bind was last seen at. */
        unsigned bindType() { return mBindType; }               /**< Returns the data type code for this bind. */
        string bindValue() { return mBindValue; }               /**< Returns the most recent value for this bind. */
        string bindName() { return mBindName; }                 /**< Returns the bind variable name as used in the SQL. */

        // Setters.
        void setBindId(unsigned val) { mBindId = val; }         /**< Sets a bind number. */
        void setBindValue(string val) { mBindValue = val; }     /**< Sets a new bind value. */
        void setBindLineNumber(unsigned val) { mBindLineNumber = val; } /**< Sets a new bind line number. */
        void setBindType(unsigned val) { mBindType = val; }     /**< Sets a new bind line number. */
        void setBindName(string val) { mBindName = val; }       /**< Sets the bind variable name. */

    protected:

    private:
        unsigned mBindId;               /**< The bind number within the SQL statement. */
        unsigned mBindLineNumber;       /**< The line number the bind details were found at. */
        unsigned mBindType;             /**< The data type for this bind (oacdty). */
        string mBindValue;              /**< The current EXEC statement's value for this bind. */
        string mBindName;               /**< The name of the bind variable in the SQL statement. */
};

#endif // TMBIND_H
