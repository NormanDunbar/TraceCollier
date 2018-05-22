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

/** @file parseBinds.cpp
 * @brief Implementation file for the tmTraceFile.parseBINDS() function.
 */

#include <algorithm>
using std::find;

#include "tmtracefile.h"
#include "gnu.h"

#ifndef USE_REGEX
    #include "utilities.h"
#endif


/** @brief Parses a "BINDS" line.
 *
 * @param thisLine const string&. The line containing, "BINDS #..."
 * @return bool. Returns true if all ok. False otherwise.
 *
 * Parses a line from the trace file. The line is expected
 * to be the BINDS \#cursor line.
 *
 * This function extracts the values for all listed binds
 * for the given cursor. The values are used to update
 * the binds map member of the appropriate tmCursor object.
 */
bool tmTraceFile::parseBINDS(const string &thisLine) {

    if (mOptions->verbose()) {
        *mDbg << "parseBINDS(" << mLineNumber << "): Entry." << endl;
    }

    // BINDS #5923197424:
    bool matchOK = true;
    string cursorID = "";

#ifdef USE_REGEX
    regex reg("BINDS\\s(#\\d+):");
    smatch match;

    // Extract the cursorID.
    if (regex_match(thisLine, match, reg)) {
            cursorID = match[1];
    } else {
        matchOK = false;
    }
#else
    cursorID = getCursor(thisLine, &matchOK);
#endif  // USE_REGEX

    if (!matchOK) {
        stringstream s;
        s << "parseBINDS(): Cannot match against BINDS # at line: "
          <<  mLineNumber << "." << endl;
        cerr << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parseBINDS(): Exit." << endl;
        }

        return false;
    }


    // Find the cursor for this exec. If it's not there
    // then it's not a traced cursor.
    map<string, tmCursor *>::iterator i = findCursor(cursorID);
    if (i == mCursors.end()) {
        // Ignore this one, depth != depth().
        if (mOptions->verbose()) {
            *mDbg << "parseBINDS(): Ignoring BINDS for cursor " << cursorID
                  << ", which has an 'out of range' depth." << endl
                  << "parseBINDS(): Exit." << endl;
        }

        return true;
    }

    // We have a valid cursor, but has it been closed?
    // This catches reuse of cursors with fewer binds, and at depth > depth().
    tmCursor *thisCursor = i->second;
    if (mOptions->verbose()) {
        *mDbg << "parseBINDS(): Found cursor: " << i->first << '.' << endl;
    }

    if (thisCursor->isClosed()) {
        if (mOptions->verbose()) {
            *mDbg << "parseBINDS(): Cursor is closed. Ignoring BINDS. " << endl;
        }

        return true;
    }


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

    // The cursor should have binds, and we are currently looking at
    // the BINDS #cursorID: line for it, so log the line number.
    thisCursor->setBindsLine(mLineNumber);

    // Ok, now we have the right stuff ready, lets read
    // *every* line relating to *all* the binds for this cursor
    // into memory for later processing.
    // We start reading from the line " Bind#0" and stop
    // after the first non-bind related line. (EXEC usually!)
    vector<string>bindData;
    string bindLine;
    bool ok = true;

    while (ok) {
        // Get next line.
        ok = readTraceLine(&bindLine);

        // Oracle has been known to throw up a bind's value line where all of
        // it is on the first line, but a second line contains the closing double quote. For example:
        //
        //  Bind#1
        //  oacdty=01 mxl=128(89) mxlc=00 mal=00 scl=00 pre=00
        //  oacflg=20 fl2=0000 frm=01 csi=46 siz=0 off=24
        //  kxsbbbfp=121c43e50  bln=128  avl=89  flg=01
        //  value="ORA-02291: integrity constraint (HEDW_EDW.CPE_PCL_ID_FK) violated - parent key not found
        //"
        // Bind#2
        // ...
        //
        // So, we need to trap this and append it to the previous line we read.
        if (bindLine == "\"") {
                string temp = bindData.back();
                cerr << "Got this: [" << temp << "]" << endl;
                temp.push_back('"');
                bindData.pop_back();
                bindData.push_back(temp);
                cerr << "Put this: [" << temp << "]" << endl;
                continue;
            }

        // Strip out those damned timestamp lines!
        if (bindLine.substr(0, 4) == "*** ")
        {
            if (mOptions->verbose()) {
                *mDbg << "parseBINDS(): Ignoring timestamp/empty line ["
                      << bindLine << ']' << endl;
            }

            continue;
        }

        // Watch out for that nasty line!
        if (bindLine.substr(2, 12) == "value= Bind#") {
            bindData.push_back("  value=");
            bindData.push_back(bindLine.substr(8));
            continue;
        }

        // Finished yet? So far, all I've ever found that
        // terminates a bind section is the EXEC for the cursor, or
        // in PL/SQL cases, the "=====...====" line for the following
        // recursive SQL statement.
        //
        // Update. This: WTF?
        //
        // BINDS #140136345356328:
        //  Bind#0
        //   oacdty=01 mxl=32(18) mxlc=00 mal=00 scl=00 pre=00
        //   oacflg=03 fl2=1000000 frm=01 csi=31 siz=32 off=0
        //   kxsbbbfp=7f7409169fe0  bln=32  avl=18  flg=05
        //   value="AAAaPmAAGAAAW71AAV"
        // XCTEND rlbk=0, rd_only=1, tim=1313055015575161
        // EXEC #140136345356328:c=0,e=941,p=0,cr=0,cu=0,mis=1,r=0,...
        //
        // I was thinking, anything can terminate the BINDS section if it
        // has no leading space? But then testing proved me wrong.
        // NVARCHAR2 and NCHAR can have value= continuation lines that begin
        // with a digit. Sigh. However, that's relatively uncommon.
        // (Famous Last Words!)

        string prefix = bindLine.substr(0, 6);
        if (prefix == "EXEC #" ||
            prefix == "======" ||
            prefix == "XCTEND" ||
            prefix == "WAIT #" ||
            prefix == "STAT #" ||
            prefix == "CLOSE " ||
            prefix == "PARSIN" ||
            prefix == "PARSE ")
        {
            break;
        }

        // Save the normal lines.
        bindData.push_back(bindLine);
    }

    // We have read one line too far. Save it for later processing.
    // I tried saving the position before each read to allow me to
    // reposition the file to the previous line, however, for some
    // trace files this fails to work correctly and we get a rogue
    // line read in next time, as opposed to the desired one. The
    // rogue bears no resemblance to either the desired line, or the
    // one prior. This solution appears to work!
    if (mOptions->verbose()) {
        *mDbg << "parseBINDS(): Pushing back this line: [" << bindLine << ']' << endl;
    }
    mUnprocessedLine = bindLine;

    // We have binds in the cursor, and we've collected the data lines
    // from the trace file. Try to extract the appropriate values.
    for (map<unsigned, tmBind *>::iterator i = thisCursor->binds()->begin();
         i != thisCursor->binds()->end();
         i++)
    {
        tmBind *thisBind = i->second;
        stringstream currentBind;
        currentBind << " Bind#" << i->first;

        if (mOptions->verbose()) {
            *mDbg << "parseBINDS(): Processing: [" << currentBind.str() << ']' << endl;
        }

        // Save the "Bind last seen at" Line number.
        //thisBind->setBindLineNumber(mLineNumber);

        // Find the first line of this bind's data and the first of the next bind's data.
        // The latter may not exist of course, if this is the final bind. The former must!
        vector<string>::iterator start_i = find(bindData.begin(), bindData.end(), currentBind.str());

        if (start_i == bindData.end()) {
            stringstream s;

            s << "parseBINDS(): Cannot locate bind data for" << currentBind.str() << endl;
            cerr << s.str();

            if (mOptions->verbose()) {
                *mDbg << s.str()
                      << "parseBINDS(): Exit." << endl;
            }

            return false;
        }


        stringstream nextBind;
        nextBind << " Bind#" << i->first + 1;
        vector<string>::iterator stop_i = find(start_i, bindData.end(), nextBind.str());

        if (mOptions->verbose()) {
            *mDbg << "parseBINDS(): start_i = [" << *start_i << ']' << endl;
            if (stop_i != bindData.end()) {
                *mDbg << "parseBINDS(): stop_i = [" << *stop_i << ']' << endl;
            } else {
                *mDbg << "parseBINDS(): stop_i = [NO MORE BINDS]" << endl;
            }
        }

        // Now we have a start and stop iterator into the bind data
        // for this particular bind. Extract the information we want.
        if (!extractBindData(start_i, stop_i, thisCursor, thisBind)) {
            stringstream s;
            s << "parseBINDS(): Failed to extract bind data for" << currentBind.str() << '.' << endl;
            cerr << s.str();

            if (mOptions->verbose()) {
                *mDbg << s.str()
                      << "parseBINDS(): Exit." << endl;
            }

            return false;
        }

        // We found the bind.
        if (mOptions->verbose()) {
            *mDbg << "parseBINDS(): Cursor: " << thisCursor->cursorId() << ": "
                  << "Bind #" << thisBind->bindId() << ": BindName: ["
                  << thisBind->bindName() << "] has value ["
                  << thisBind->bindValue() << ']' << endl;
        }
    }

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parseBINDS(): Exit." << endl;
    }

    return true;
}


/** @brief Parses a vector of lines relating to a single bind variable to extract the value etc.
 *
 * @param start const vector<string>::iterator. Start of lines to scan.
 * @param stop const vector<string>::iterator. Just after the last line to scan.
 * @param thisCursor tmCursor*. The tmCursor object who's data we are extracting.
 * @param thisBind tmBind*. The tmBind object who's data we are extracting.
 * @return bool. Returns true for success, false otherwise.
 *
 * Parses a vector of lines, read in from the trace file,  which relate to a single
 * bind variable. The value for this current execution's bind variable is extracted and will
 * be used as a substitute for the variable name in the report file when we hit the EXEC for
 * this cursor.
 *
 * In normal use, there should be a value. However, if a bind is used more than once, then
 * the second and subsequent uses will have no bind details at all, only a single line of
 * text stating "No oacdef for this bind" in which case we need to look up the other binds
 * in the map to find the value to use. Oracle (currently) guarantees that one will exists
 * and that it will have been read already as it appears in the trace file prior to the current
 * bind.
 *
 * Bind Data Fields of interest:
 *
 * Oacdty = Data type. Types of interest (so far) are:
 *          1 = VARCHAR2 or NVARCHAR2. VARCHAR2 values are in double quotes, NVARCHAR2 values are hex dumps.
 *          2 = NUMBER. Beware if the value is ### which I think is used for an OUT parameter.
 *         11 = ROWID.
 *         23 = RAW. A string of Hex digits.
 *         25 = Unhandled data type.
 *         29 = Unhandled data type.
 *         96 = NCHAR. Dumped as Hex digits.
 *        102 = PL/SQL OUT REF_CURSOR.
 *        123 = A buffer. A VARRAY. Usually seen in DBMS_OUTPUT.GET_LINES(:LINES, :NUM_LINES). The :LINES bind will be type 123.
 * Avl = Average length. Sometimes means the number of characters in the ASCII representation of the bind's value. Zero indicates a NULL
 *       or a PL/SQL OUT parameter.
 * Value = The bind's value. May be missing, or "invalid".
 * Mxl = Maximum length, but is not reliable. It's the internal format's maximum length.
 *
 */
bool tmTraceFile::extractBindData(const vector<string>::iterator start, const vector<string>::iterator stop, tmCursor *thisCursor, tmBind *thisBind) {

    if (mOptions->verbose()) {
        *mDbg << "extractBindData(): Entry." << endl
              << "extractBindData(): Extracting data for Bind #"
              << thisBind->bindId() << '.' << endl;
    }

    // Storage for the data we are extracting from the vector.
    unsigned dataType = 0;
    unsigned averageLength = 0;
    string value = "";
    vector<string>::const_iterator valueStartsHere = stop;

    // Flags.
    string::size_type oacdtyPos = 0;
    string::size_type avlPos = 0;
    string::size_type noOacdefPos = 0;
    string::size_type valuePos = 0;


    // Parse the bindData vector for the data we want.
    for (vector<string>::const_iterator i=start;
        i != stop;
        i++)
    {
        if (mOptions->verbose()) {
           *mDbg << "extractBindData(): Scanning line: [" << *i << ']' << endl;
        }

        // Set the flags.
        oacdtyPos = i->find("oacdty=");
        avlPos = i->find("avl=");
        noOacdefPos = i->find("No oacdef");
        valuePos = i->find("value=");

        //----------------------------------------------------------------
        // No oacdef?
        //----------------------------------------------------------------
        if (noOacdefPos != string::npos) {
           // Need to find and copy from a previous bind.
           // Then we are done.
            if (mOptions->verbose()) {
               *mDbg << "extractBindData(): 'No oacdef' found." << endl;
            }

            // Get the cursor's bind map.
            map<unsigned, tmBind *> *bindMap = thisCursor->binds();

            // Lookup the desired binds. It has to be one that
            // has come before this one, so we don't look at them all.
            for (unsigned i = 0; i < thisBind->bindId(); i++) {
                map<unsigned, tmBind *>::iterator bi = bindMap->find(i);
                if (bi != bindMap->end()) {
                    // Find the bind with the same name.
                    if (bi->second->bindName() == thisBind->bindName()) {
                        // Copy  the value across.
                        thisBind->setBindValue(bi->second->bindValue());
                        // Update the tmBind object with the data type.
                        thisBind->setBindType(bi->second->bindType());

                        if (mOptions->verbose()) {
                            *mDbg << "extractBindData(): Bind#" << thisBind->bindId()
                                  << " has same data as Bind#" << bi->second->bindId()
                                  << " Bind name [" << bi->second->bindName() << "]." << endl
                                  << "extractBindData(): Exit." << endl;
                        }

                        // We have the bind's value, we are done here.
                        return true;
                    }
                }
            }

            // Hmm. We exited the loop without finding the other bind
            // that we were looking for. Bad news time.

            stringstream s;

            s << "extractBindData(): Bind#" << thisBind->bindId()
              << " is a copy of another bind variable. However"
              << " extractBindData() was unable to find it." << endl;

            cerr << s.str();

            if (mOptions->verbose()) {
                *mDbg << s.str()
                      << "extractBindData(): Exit." << endl;
            }

            return false;
        }

        //----------------------------------------------------------------
        // Value=?
        //----------------------------------------------------------------
        if (valuePos != string::npos) {
           // Save the value iterator. We extract
           // the actual value later.
           valueStartsHere = i;

           if (mOptions->verbose()) {
              *mDbg << "extractBindData(): 'Value=' found." << endl;
           }

           continue;
        }

        //----------------------------------------------------------------
        // Oacdty?
        //----------------------------------------------------------------
        if (oacdtyPos != string::npos) {

           if (mOptions->verbose()) {
              *mDbg << "extractBindData(): 'Oacdty=' found." << endl;
           }

           if (!extractNumber(i, oacdtyPos + 6, dataType)) {
                stringstream s;
                s << "extractBindData(): Failed to extract Data Type (OACDTY) for bind." << endl;
                cerr << s.str();

                if (mOptions->verbose()) {
                    *mDbg << s.str()
                          << "extractBindData(): Exit.";
                }

                return false;
           }

           if (mOptions->verbose()) {
              *mDbg << "extractBindData(): 'Data Type is " << dataType << '.' << endl;
           }

           continue;
        }

        //----------------------------------------------------------------
        // Avl?
        //----------------------------------------------------------------
        if (avlPos != string::npos) {

           if (mOptions->verbose()) {
              *mDbg << "extractBindData(): 'Avl=' found." << endl;
           }

           if (!extractNumber(i, avlPos + 3, averageLength)) {
                stringstream s;
                s << "extractBindData(): Failed to extract Average Length (AVL) for bind." << endl;
                cerr << s.str();

                if (mOptions->verbose()) {
                    *mDbg << s.str()
                          << "extractBindData(): Exit.";
                }

                return false;
           }

           if (mOptions->verbose()) {
              *mDbg << "extractBindData(): 'Average Length is " << averageLength << '.' << endl;
           }

           continue;
        }

        // Update the tmBind object with the data type.
        thisBind->setBindType(dataType);

    }

    // PL/SQL or not, a data type 102 is definitely a REF_CURSOR.
    if (dataType == 102) {
        thisBind->setBindValue("REF_CURSOR");
        if (mOptions->verbose()) {
            *mDbg << "extractBindData(): Bind variable: " << thisBind->bindId()
                  << "('" << thisBind->bindName() << "') for cursor: "
                  << thisCursor->cursorId() << ", has dataType 102. "
                  << "Setting value to 'REF_CURSOR'." << endl
                  << "extractBindData(): Exit." << endl;
        }

        // I am done here.
        return true;
    }

    // If averageLength is zero, or, we didn't find a "value="
    // then this is most likely an OUT parameter for PL/SQL,
    // OR, a NULL value for a BIND in plain SQL.
    if (valuePos == string::npos ||
        averageLength == 0) {
            // Find our current cursor's Oracle Action Code.
            if (thisCursor->commandType() != COMMAND_PLSQL) {
                // In SQL, no 'value=' means a NULL value.
                thisBind->setBindValue("NULL");
            } else {
                // PL/SQL = buffer, OUT parameter etc.
                // Use the bind variable's name as it's value.
                thisBind->setBindValue(thisBind->bindName());
            }

            if (mOptions->verbose()) {
                *mDbg << "parseBindData(): Suspected OUT PL/SQL parameter found, or," << endl
                      << "parseBindData(): NULL value for bind variable found." << endl
                      << "parseBindData(): Exit." << endl;
            }

            return true;

        }

    // We have a good bind, with a value present, extract it.
    if (!extractBindValue(valueStartsHere, thisBind)) {
        // Damn!
        stringstream s;
        s << "extractBindData(): Call to extractBindValue() Failed to extract bind value for 'Bind#"
          << thisBind->bindId() << '\'' << endl;
        cerr  << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "extractBindData(): Exit." << endl;
        }

        return false;
    }

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "extractBindData(): Exit." << endl;
    }

    return true;
}


/** @brief Extracts a numeric value located in a string, between the '=' and the following space.
 *
 * @param i vector<string>::const_iterator. Iterator pointing at the string.
 * @param equalPos const unsigned. Where the '=' is found in the string.
 * @param result unsigned&. Variable to receive the result.
 * @return bool. True is success. False is otherwise.
 */
bool tmTraceFile::extractNumber(vector<string>::const_iterator i, const unsigned equalPos, unsigned &result) {

   if (mOptions->verbose()) {
      *mDbg << "extractNumber(): Entry." << endl;
   }

   try {
       // Stoul() extracts a number from a string. It stops after the first non numeric character.
       // "E" or "e" might be a problem if we hit one though!
       result = stoul(i->substr(equalPos + 1), NULL, 10);
   } catch (exception &e) {
       stringstream s;
       s << "extractNumber(): Exception: " << e.what() << endl
         << "extractNumber(): Failed to extract numeric data for bind." << endl;
       cerr << s.str();

       if (mOptions->verbose()) {
           *mDbg << s.str()
                 << "extractNumber(): Exit.";
       }

       return false;
   }

   if (mOptions->verbose()) {
      *mDbg << "extractNumber(): 'Result is "
            << result << '.' << endl
            << "extractNumber(): Exit." << endl;
   }

   return true;
}


/** @brief Extracts a Hex value located in a string, between the '=' and the end of the string.
 * data are stored in the tmBind object in ASCII format, between single quotes.
 *
 * @param i vector<string>::const_iterator. Iterator pointing at the string.
 * @param equalPos const unsigned. Where the '=' is found in the string.
 * @param result string&. Variable to receive the result.
 * @return bool. True is success. False is otherwise.
 */
bool tmTraceFile::extractHex(vector<string>::const_iterator i, const unsigned equalPos, string &result) {


    if (mOptions->verbose()) {
       *mDbg << "extractHex(): Entry." << endl;
    }

    // Initialise result string.
    result.clear();
    result.push_back('\'');

    // Convert a pile of hex values into ASCII by simply ignoring the
    // leading '0' and taking the following hex as a character.
    // Assumes the string starts with a '0' and a space. Bad idea?
    string temp = i->substr(equalPos + 1);
    unsigned digits;

    // Stoul() stops at the first non digit.
    while (true) {
        // What are we looking at right now?
        // Handy if I need to debug stuff.
        //if (mOptions->verbose()) {
        //   *mDbg << "extractHex(): Extracting digits [" << temp << ']' << endl;
        //}

        // Done yet?
        if (temp.empty()) {
            break;
        }

        // Should be leading digits now.
        size_t spaceHere;
        try {
            digits = stoul(temp, &spaceHere , 16);
        } catch (exception &e) {

            stringstream s;
            s << "extractHex(): Exception: " << e.what() << endl
              << "extractHex(): Failed to extract hex data from '" << temp << '\'' << endl;
            cerr << s.str();

            if (mOptions->verbose()) {
                *mDbg << s.str()
                      << "extractHex(): Exit.";
            }

            return false;
        }

        // We ignore zero.
        if (digits) {
            char cc = digits;
            result.push_back(cc);
        }

        // Strip off what we got.
        temp = temp.substr(spaceHere + 1);
    }

    // Terminate result string.
    result.push_back('\'');

    if (mOptions->verbose()) {
       *mDbg << "extractHex(): Result is "
             << result << '.' << endl
             << "extractHex(): Exit." << endl;
    }

    return true;
}



/** @brief Extract a binds actual value as a string.
 *
 * @param i vector<string>::const_iterator. The line we are extracting a value from.
 * @param thisBind tmBind*. The tmBind object who's value we are extracting.
 * @return bool. True means all ok. False means problems.
 *
 * The bind data types are:
 *
 *   1 = VARCHAR2 or NVARCHAR2. VARCHAR2 values are in double quotes, NVARCHAR2 values are hex dumps.
 *   2 = NUMBER. Beware if the value is ### which I think is used for an OUT parameter.
 *   8 = LONG. A string of Hex digits? Maybe?
 *  11 = ROWID. Just the ROWID, no quotes etc.
 *  12 = DATE.
 *  23 = RAW. A string of Hex digits? Maybe?
 *  24 = LONG RAW. A string of Hex digits? Maybe?
 *  25 = Unhandled data type.
 *  29 = Unhandled data type.
 *  69 = ROWID. See Type 11. Oracle document 69 as ROWID. Tracefiles show type 11. Go figure!
 *  96 = NCHAR. Dumped as Hex digits.
 * 100 = BINARY_FLOAT.
 * 101 = BINARY_DOUBLE.
 * 108 = User-defined type (object type, VARRAY, nested table)
 * 111 = REF.
 * 112 = CLOB or NCLOB.
 * 113 = BLOB.
 * 114 = BFILE.
 * 123 = A buffer. VARRAY? Usually seen in DBMS_OUTPUT.GET_LINES(:LINES, :NUM_LINES). The :LINES bind will be type 123.
 * 180 = TIMESTAMP.
 * 181 = TIMESTAMP WITH TIME ZONE.
 * 182 = INTERVAL YEAR TO MONTH.
 * 183 = INTERVAL DAY TO SECOND.
 * 208 = UROWID.
 * 231 = TIMESTAMP WITH LOCAL TIME ZONE.
 */
bool tmTraceFile::extractBindValue(vector<string>::const_iterator i, tmBind *thisBind) {

   if (mOptions->verbose()) {
      *mDbg << "extractBindValue(): Entry." << endl
            << "extractBindValue(): Processing Bind#" << thisBind->bindId() << '.' << endl
            << "extractBindValue(): Extracting Hex from [" << *i << ']' << endl;
   }

   unsigned equalPos = i->find("=");
   unsigned quotePos = i->find("\"");

   // We need the data type.
   unsigned dataType = thisBind->bindType();
   switch (dataType) {
       //----------------------------------------------------------------------
       // VARCHAR2 has a value="quoted string".
       // NVARCHAR2 has hex data without quotes.
       // Both drop into type 96, NCHAR, which is always hex data, but checks
       // for a VARCHAR2 double quote first.
       //----------------------------------------------------------------------
       case 1: // VARCHAR2 or NVARCHAR2.
                // Drop in below.

       //----------------------------------------------------------------------
       // NCHAR is always hex data. In case we are here from VARCHAR2 above, we
       // check for a leading double quote first.
       //----------------------------------------------------------------------
       case 96: // NCHAR.
            if ((equalPos + 1) == quotePos) {
               // This is a VARCHAR2. We have a double-quoted string.
               string thisValue = i->substr(quotePos);
               thisValue.at(0) = '\'';
               thisValue.at(thisValue.length() - 1) = '\'';
               thisBind->setBindValue(thisValue);
            } else {
               // This is an NCHAR or NVARCHAR2, extract the hex data.
               string thisValue;
               if (extractHex(i, equalPos, thisValue)) {
                   thisBind->setBindValue(thisValue);
               } else {
                   stringstream s;
                   s << "extractBindValue(): Failed to extract Hex." << endl;
                   cerr << s.str();

                   if (mOptions->verbose()) {
                       *mDbg << s.str()
                             << "extractBindValue(): Exit." << endl;
                   }

                   return false;
               }
           }
           break;

       //----------------------------------------------------------------------
       // Numbers are always numerical and on the first line. If the number is
       // "###" though, it's an output bind for PL/SQL.
       //----------------------------------------------------------------------
       case 2: // NUMBER.
           thisBind->setBindValue(i->substr(equalPos + 1));
           if (thisBind->bindValue() == "###") {
               thisBind->setBindValue(thisBind->bindName());
           }
           break;

       //----------------------------------------------------------------------
       // ROWIDs and the two unhandled data types (Oracle's words not mine) are
       // simple extractions of the remaining data on the line.
       //----------------------------------------------------------------------
       case 11: // ROWID. Convert from char to rowid. Actually seen in trace files.
       case 69: // ROWID. Convert from char to rowid. From the 11gr2 docs.
           thisBind->setBindValue("CHARTOROWID('" + i->substr(equalPos + 1) + "')");
           break;

       case 25: // UNHANDLED DATA TYPE. (Drop in below).
       case 29: // UNHANDLED DATA TYPE.
           thisBind->setBindValue(i->substr(equalPos + 1));
           break;

       //----------------------------------------------------------------------
       // RAW does what exactly? ***** TODO ??*****
       //----------------------------------------------------------------------
       case 23: // RAW.
           thisBind->setBindValue(i->substr(equalPos + 1));
           break;

       //----------------------------------------------------------------------
       // VARRAY. Usually a PL/SQL return or OUT parameter. Just use the name.
       //----------------------------------------------------------------------
       case 108: // A Data Buffer.valueStartsHere. From the 11gr2 docs.
       case 123: // A Data Buffer.valueStartsHere. Actually seen in trace files.
           thisBind->setBindValue(thisBind->bindName());
           break;

       //----------------------------------------------------------------------
       // OOPS! I assume just copy the data from the value line will suffice if
       // we hit an unknown data type. Time will tell.
       //----------------------------------------------------------------------
       default: // I have no idea what you are!
           thisBind->setBindValue(i->substr(equalPos + 1));
           break;

        // If we reach here, we are done.
        return true;
   }


   // Looks like a good parse.
   if (mOptions->verbose()) {
      *mDbg << "extractBindValue(): Result = " << thisBind->bindValue() << endl
            << "extractBindValue(): Exit." << endl;
   }

    return true;
}
