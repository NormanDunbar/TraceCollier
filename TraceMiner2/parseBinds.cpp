/** @file parseBinds.cpp
 * @brief Implementation file for the tmTraceFile.parseBINDS() function.
 */

#include "tmtracefile.h"
#include <vector>
using std::vector;


/** @brief Parses a "BINDS" line.
 *
 * Parses a line from the trace file. The line is expected
 * to be the BINDS #cursor line.
 *
 * This function extracts the values for all listed binds
 * for the given cursor. The values are used to update
 * the binds map member of the appropriate tmCursor object.
 *
 * Returns true if all ok. False otherwise.
 */
bool tmTraceFile::parseBINDS(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parseBINDS(): Entry." << endl;
    }

    // BINDS #5923197424:
    regex reg("BINDS\\s(#\\d+):");
    smatch match;

    // Extract the cursorID.
    if (!regex_match(thisLine, match, reg)) {
        stringstream s;
        s << "parseBINDS(): Cannot match regex against BINDS # at line: "
          <<  mLineNumber << "." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parseBINDS(): Exit." << endl;
        }

        return false;
    }

    string cursorID = match[1];

    // Find the cursor for this exec. If it's not there
    // then it's not a depth=0 cursor.
    map<string, tmCursor *>::iterator i = findCursor(cursorID);
    if (i == mCursors.end()) {
        // Ignore this one, depth != 0.
        if (mOptions->verbose()) {
            *mDbg << "parseBINDS(): Ignoring BINDS with non-zero depth." << endl
                  << "parseBINDS(): Exit." << endl;
        }

        return true;
    }

    // We have a valid cursor.
    tmCursor *thisCursor = i->second;

    // Any binds?
    unsigned bindCount = thisCursor->bindCount();
    if (!bindCount) {
        // Weird. No binds required, but we have binds anyway.
        // Barf!
        stringstream s;
        s << "parseBINDS(): Cursor " << cursorID
          << " should have no binds, "
          << "but the trace file says otherwise at line: "
          << mLineNumber << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str() << endl
                  << "parseBINDS(): Exit." << endl;
        }

        return false;
    }

    // We have binds in the cursor, and we've found them
    // in the trace file. Extract the appropriate values.
    for (map<unsigned, tmBind *>::iterator i = thisCursor->binds()->begin();
         i != thisCursor->binds()->end();
         i++) {
             tmBind *thisBind = i->second;

             // Let's see if we can find the "value=" line for this bind.
             if (!parseBindData(thisBind)) {
                // Failed to parse. Hmmm.
                stringstream s;
                s << "parseBINDS(): Cursor " << cursorID
                  << ", Bind#" << thisBind->bindId()
                  << ": Failed to parse a value."
                  << endl;
                cerr << s.str();

                if (mOptions->verbose()) {
                    *mDbg << s.str()
                          << "parseBINDS(): Exit." << endl;
                }

        return false;
             }
    }



    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parseBINDS(): Exit." << endl;
    }

    return true;
}


/** @brief Parses a number of lines relating to a single bind variable..
 *
 * Parses a number of lines from the trace file which define the details
 * about a single bind variable.
 *
 * On entry, for each bind variable we read a single line and check that it
 * is the "Bind#n" line for the correct bind, otherwise we barf.
 *
 * Returns true if all ok. False otherwise.
 */
bool tmTraceFile::parseBindData(tmBind *thisBind) {

    if (mOptions->verbose()) {
        *mDbg << "parseBindData(): Entry." << endl
              << "parseBindData(): Bind #" << thisBind->bindId() << endl;
    }

    string thisLine;
    bool ok;

    // We need to read one line here to position on the "Binds#n" line.
    ok = readTraceLine(&thisLine);
    if (!ok) {
        stringstream s;
        s << "parseBindData(): Trace file read error." << endl;
        cerr  << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parseBindData(): Exit." << endl;
        }

        return false;
    }

    // Did we read the correct line?
    // " Bind#n"
    regex reg("\\sBind#(\\d+)") ;
    smatch match;

    if (!regex_match(thisLine, match, reg)) {
        stringstream s;
        s << "parseBindData(): Expecting 'Bind#" << thisBind->bindId()
          << "', found [" << thisLine << ']' << endl;
        cerr  << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parseBindData(): Exit." << endl;
        }

        return false;
    }

    // Ok, collect all the stuff for this bind.
    // Save it in a vector<string> as we might need
    // to split that famous "value= Bind#n" line that turns up
    // from time to time.
    // We also need to push back the EXEC or next bind line.
    vector<string> bindStuff;
    unsigned long pos;

    // Ok set true above.
    while (ok) {
        // Keep reading lines until we find:
        // An EXEC line, or
        // The next Bind#n line, or
        // No oacdef for this bind, or
        // EOF.

        // Save previous line.
        bindStuff.push_back(thisLine);

        // Get current position.
        pos = mIfs->tellg();

        if (mOptions->verbose()) {
            *mDbg << "parseBindData(): Pos: " << pos << endl;
        }

        // Not done, read on MacDuff!
        ok = readTraceLine(&thisLine);

        // Finished yet?
        string prefix = thisLine.substr(0, 6);
        if (prefix == "EXEC #" ||
            prefix == " Bind#" ||
            prefix == "  No o" ||
            mIfs->eof()){
            break;
        }

        // Found that funny line?
        // Assume no value to extract, and we need to position backwards to the Bind#n.
        if (thisLine == "  value= Bind#") {
            if (mOptions->verbose()) {
                *mDbg << "parseBindData(): Found this line [" << thisLine << ']' << endl;
            }
        }
    }

    mIfs->seekg(pos, mIfs->beg);
    if (mOptions->verbose()) {
        // Get current position.
        pos = mIfs->tellg();
        *mDbg << "parseBindData(): Loop exit: Pos: " << pos << endl;
    }


    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parseBindData(): Exit." << endl;
    }

    return true;
}
