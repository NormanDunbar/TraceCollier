/** @file parseParse.cpp
 * @brief Implementation file for the tmTraceFile.parsePARSE() function.
 */

#include "tmtracefile.h"

/** @brief Parses a "PARSE" line.
 *
 * Parses a line from the trace file. The line is expected
 * to be the PARSE #cursor line.
 *
 * The tmCursor associated with this PARSE is found, and updated to the new
 * source file line number. Only the most recent PARSE is stored for each tmCursor.
 *
 * Returns true if all ok. False otherwise.
 */
bool tmTraceFile::parsePARSE(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parsePARSE(): Entry." << endl;
    }

    // PARSE #5924310096:c=0,e=28,p=0,cr=0,cu=0,mis=0,r=0,dep=0,og=4,plh=1388734953,tim=526735705337
    regex reg("PARSE\\s(#\\d+).*?dep=(\\d+).*");
    smatch match;

    // Extract the cursorID, the length and the depth.
    if (!regex_match(thisLine, match, reg)) {
        stringstream s;
        s << "parsePARSE(): Cannot match regex against PARSE # at line: "
          <<  mLineNumber << "." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parsePARSE(): Exit." << endl;
        }

        return false;
    }

    // Found it!
    // Extract the goodies! We definitely have all three matches.
    // Match[0] = the whole line!
    // Match[1] = "#12345678"
    // Match[2] = "0"

    // No validation necessary, they are digits etc, as required.

    // The SQL starts on the following line, not this one!
    string cursorID = match[1];
    unsigned depth = stoul(match[2], NULL, 10);

    // We only care about user level SQL, so only depth 0.
    if (depth) {
        // Ignore this one.
        if (mOptions->verbose()) {
            *mDbg << "parsePARSE(): Ignoring PARSE with dep=" << depth << '.' << endl
                  << "parsePARSE(): Exit." << endl;
        }

        return true;
    }


    // Find the existing cursor.
    map<string, tmCursor *>::iterator i = findCursor(cursorID);

    // Found?
    if (i != mCursors.end()) {
        // Yes. Thankfully! Update the PARSE line number.
        // i is an iterator to a pair<string, tmCursor *>
        // So i->first is the string.
        // And i->second is the tmCursor pointer.
        i->second->setSQLParseLine(mLineNumber);
    } else {
        // Not found. Oh dear!
        stringstream s;
        s << "parsePARSE(): Found PARSE for cursor " << cursorID
          << " but not found in existing cursor list." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parsePARSE(): Exit." << endl;
        }

        return false;
    }

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parsePARSE(): Exit." << endl;
    }

    return true;
}


