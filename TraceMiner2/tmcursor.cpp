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

#include "tmcursor.h"
#include "gnu.h"

#ifndef USE_REGEX
#include "utilities.h"
#endif // USE_REGEX

/** @file tmcursor.cpp
 * @brief Implementation file for the tmCursor object.
 */


/** @brief Constructor for tmCursor object.
 *
 * @param	id std::string. The cursorID including leading '#'.
 * @param	sqlSize unsigned. The length of the SQL text.
 * @param	sqlLine unsigned. The line in the trace file where the SQL statement begins.
 * @return	None.
 */
 tmCursor::tmCursor(string id, unsigned sqlSize, unsigned sqlLine) {
    mCursorId = id;
    mSQLLineNumber = sqlLine;
    mSQLSize = sqlSize;
    mSQLText = "";
    mSQLParseLine = 0;
    mBindCount = 0;
    mBindsLine = 0;
    mCommandType = 0;
    mClosed = false;

}

/** @brief Destructor for tmCursor object.
 */
tmCursor::~tmCursor() {
    // Anything to do to kill a tmCursor?
    // Of course there is, get shot of all the binds in the map.
    //cerr << endl << "Deleting Cursor: " << mCursorId; // Debugging.
    cleanUp();
}


/** @brief Cleans up on destruction of a tmCursor and on changing the SQL.
 *
 * When a tmCursor is destroyed, we must clean up all the assigned tmBinds
 * from the map. This function is also called when the SQL Text is changed
 * on reuse of the cursor. That way we don't leak tmBinds all over the place
 * when a cursor is closed and reused.
 */
void tmCursor::cleanUp() {

    //short bindCount = 0;
    if (mBinds.size()) {
        for (map<unsigned, tmBind *>::iterator i = mBinds.begin(); i != mBinds.end(); ++i) {
            // Dump the bind details for debugging.
            //cerr << "cleanUp(): Bind number: " << bindCount++ << endl;
            //cerr << *(i->second);

            // Destruct this particular tmBind.
            delete i->second;
        }

        // Finally, clear the map.
        mBinds.clear();
    }
}


/** @brief Allows a tmCursor to be streamed to an ostream.
 *
 * @return	ostream & (reference).
 *
 * This function/operator allows a tmCursor to stream itself
 * out to an ostream. It does not, at present, stream out
 * a cursor's binds.
 *
 * Only used for debugging and verbose output.
 */
ostream &operator<<(ostream &out, const tmCursor &cursor) {
    out << "CursorID: " << cursor.mCursorId << endl
        << "SQL Line Number: " << cursor.mSQLLineNumber << endl
        << "SQL Text Length: " << cursor.mSQLSize << endl
        << "SQL Parse Line: " << cursor.mSQLParseLine << endl
        << "Bind Count: " << cursor.mBindCount << endl
        << "Final \"BINDS " << cursor.mCursorId << ":\" Line for this cursor: " << cursor.mBindsLine << endl
        << "Command Type: " << cursor.mCommandType << endl
        << "SQL Text = [" << cursor.mSQLText << "]" << endl
        << "Closed? " << cursor.mClosed << endl;

    // If we have any binds, print them out.
    if (cursor.mBinds.size()) {
        for (map<unsigned, tmBind *>::const_iterator i = cursor.mBinds.begin(); i != cursor.mBinds.end(); ++i) {
            out << *(i->second);
        }
    }

    out << endl;

    return out;
}


/** @brief Updates the SQL statement & binds when the SQL changes.
 *
 * @param val string. The (new) SQL statement for this tmCursor.
 *
 * When a cursor gets a new SQL statement, the binds are extracted
 * and a tmBind map set up where the key is the bind name (:xxx) and
 * the rest is the bind stuff itself.
 *
 * If a statement uses the same bind more than once, that's acceptable
 * as the bind map is keyed on the bind number not the name.
 */
void tmCursor::setSQLText(string val) {

    // Assign the (new) SQL statement.
    mSQLText = val;

    // Build the binds list.
    buildBindMap(val);

}


/** @brief Initialises the list of Bind objects when the SQL changes.
 *
 * @param sql const string&. The full text of the SQL statement we are extracting binds from.
 * @return bool.
 *
 * This function does the hard work of extracting the bind variables
 * when a cursor has a (new) SQL statement assigned. Old ones are removed
 * and cleaned up.
 *
 * A return of true indicates success, false otherwise.
 */
bool tmCursor::buildBindMap(const string &sql) {

    // If we have any binds, clean them out.
    if (mBinds.size()) {
        cleanUp();
    }

    // Now, hunt down and extract any binds.
    // Beware, ":=" doesn't constitute a bind variable!
    string thisSQL = sql;
    string bindName = "";

#ifdef USE_REGEX
    // Binds are a ":" followed by one or more underscores,
    // letters and/or digits. Double quotes optional.
    // This regex finds valid binds.
    // regex reg("(:\"?\\w+\"?)");

    // ISSUE #9. The above regex finds incorrect binds if there is not a space,
    // tab or newline etc prior to the colon. Eg. TO_DATE('dd:mm:yy'...) has
    // two binds :mm and :yy when it doesn't have any!
    regex reg("[:space: (=,+-/*](:\"?\\w+\"?)");
    smatch match;
#else
    string::size_type colonPos = 0;
#endif // USE_REGEX

    // Oracle numbers binds from 0, in the order that
    // they appear in the SQL.
    unsigned bindID = 0;

    // Scan the SQL looking for binds.
    while (true) {

#ifdef USE_REGEX
    if (!regex_search(thisSQL, match, reg)) {
        break;
    }
        // extract the bind name, including the colon.
        bindName = match[1];
        cerr << "Found bind '" << bindName << "'" << endl;
#else
        // All of this code *should* have been in the extractNextBind function,
        // but for some unknown, and undeterminable reason, looking for a colon
        // in the string, passed as a parameter (reference, pointer etc) "found"
        // one at the position 0, and at the position of EVERY space thereafter.
        // Weird! Hence, I must look for the colon here, and extract the name
        // in the function. Windows and Linux by the way.
        colonPos = thisSQL.find(':', colonPos + bindName.length());
        if (colonPos == string::npos) {
            // No more colons anymore!
            break;
        }

        // Don't consider PL/SQL assignment as a bind variable.
        if (thisSQL.at(colonPos + 1) == '=') {
            colonPos += 2;
            continue;
        }

        // Don't consider anything as a bind variable unless the
        // colon is followed by a letter, digit or underscore.
        if ((thisSQL.at(colonPos + 1) != '_') &&
            (!isdigit(thisSQL.at(colonPos + 1))) &&
            (!isalpha(thisSQL.at(colonPos + 1))))
        {
            colonPos += 2;
            continue;
        }

        // ISSUE #9.
        // Don't consider this to be a bind variable if it is not
        // preceded by a space, or tab or newline etc. You cannot have a
        // bind immediately followed by another "SELECT :a:b ..." for eg.
        if (colonPos > 0) {
            if ((thisSQL.at(colonPos - 1) != ',') &&
               (thisSQL.at(colonPos - 1) != ' ') &&
               (thisSQL.at(colonPos - 1) != '\t') )
            {
                cerr << "Bind at position " << colonPos << " is not a bind." << endl;
                cerr << "It  is preceeded by a '" << thisSQL.at(colonPos - 1) << "'" << endl;
                colonPos++;
                continue;
            }
        }


        // Ok, extract a bind variable name.
        if (!extractBindName(thisSQL, colonPos, bindName)) {
            cerr << "buildBindMap(): extractBindName() failed." << endl;
            return false;
        }
#endif // USE_REGEX

        // Save the Bind details.
        tmBind *thisBind = new tmBind(bindID, bindName);

        // An iterator for the insert into the bind map. AKA where are we?
        pair<map<unsigned, tmBind *>::iterator, bool> exists;

        // Stash this new bind. If the bind exists, update it. Maps are weird!
        exists = mBinds.insert(pair<unsigned, tmBind *>(bindID, thisBind));

        // If the bool is false, the insert failed - already there.
        // So we simply barf!.
        //
        // Exists is a pair<mBinds::iterator, bool>. It indexes the map at the
        // position of the inserted or already existing cursor data.
        // Exists.second is the bool. True=inserted, False=already exists.
        // Exists.first is an iterator (pseudo pointer) to a pair <string, tmBind *>.
        // Exists.first->first is the string, aka the bindName.
        // Exists.first->second is the tmBind pointer.
        //
        // Phew!

        // So, after all that, did we insert or find our bind?
        if (!exists.second) {
            // This should never happen!
            cerr << "Already exists: " << bindName << " with bind id "
                 << bindID << endl;
            delete thisBind;
            return false;
        }

        // Search the rest of the SQL text next time around.
#ifdef USE_REGEX
        thisSQL = match.suffix();
#endif  // USE_REGEX

        bindID++;
    }   // end while

    mBindCount = bindID;

    // Looking good!
    return true;
}
