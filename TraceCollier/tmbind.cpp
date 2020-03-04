/*
 * MIT License
 *
 * Copyright (c) 2017 Norman Dunbar
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "tmbind.h"

/** @file tmbind.cpp
 * @brief Implementation file for the tmBind object.
 */


/** @brief Constructor for a tmBind object.
 *
 * @param id unsigned. The positional bind number. The first  bind in a SQL statement is bind 0, and so on.
 * @param name std::string. The extracted bind variable name. May be wrapped in double quotes, maybe not.
 *
 */
tmBind::tmBind(unsigned id, string name) {

    mBindId = id;
    mBindLineNumber = 0;
    mBindType=0;
    mBindValue = "";
    mBindName = name;
}

/** @brief Destructor for a tmBind object.
 */
tmBind::~tmBind()
{
    //dtor
}


/** @brief Allows a tmBind to be streamed to an ostream.
 *
 * @param out ostream&. The stream to output tmBind details to.
 * @param bind tmBind&. The tmBind to be streamed.
 * @return ostream&. The same as the input stream.
 *
 * This function/operator allows a tmBind to stream itself
 * out to an ostream.
 */
ostream &operator<<(ostream &out, const tmBind &bind) {
    out << endl
        << "BindID: " << bind.mBindId << endl
        << "Bind Line Number: " << bind.mBindLineNumber << endl
        << "Bind Type: " << bind.mBindType << endl
        << "Bind Name: " << bind.mBindName << endl
        << "Bind Value: " << bind.mBindValue << endl;
    return out;
}

