/** @file parseBinds.cpp
 * @brief Implementation file for the tmTraceFile.parseBINDS() function.
 */

#include "tmtracefile.h"

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
        *mDbg << "parseBINDS(" << mLineNumber << "): Entry." << endl;
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
        *mDbg << "parseBindData(" << mLineNumber << "): Entry." << endl
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

    // Save the "Bind last seen at" Line number.
    thisBind->setBindLineNumber(mLineNumber);

    // Ok, collect all the stuff for this bind.
    // Save it in a vector<string> as we might need
    // to split that famous "value= Bind#n" line that turns up
    // from time to time.
    // We also need to push back the EXEC or next bind line.
    vector<string> bindStuff;
    unsigned long pos;
    bool foundStrangeLine = false;

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

            // Ok, problem. We need to read the " Bind#" part on the next read
            // so we need to position back to the space before the 'B'. However,
            // are we on Windows or Unix? In other words do we go back one character
            // for Unix and two for Windows? How to determine which one to use?
            // Don't! Just set pos to pos + 8 which is the desired position.
            pos += 8;
            thisLine = thisLine.substr(0, 8);
            foundStrangeLine = true;
        }
    }

    // We have read past where we want to be, so set the position
    // back to the start of the line we just read in.
    mIfs->seekg(pos, mIfs->beg);
    if (mOptions->verbose()) {
        // Get current position.
        pos = mIfs->tellg();
        *mDbg << "parseBindData(): Loop exit: Pos: " << pos << endl;
    }

    // We need to adjust the line counter too, but only
    // if we didn't find that strange "value= Bind#n" line.
    mLineNumber -= (foundStrangeLine ? 0 : 1);


    // Now, extract the Bind data from the vector.
    if (!extractBindData(&bindStuff, thisBind)) {
        // Damn!
        stringstream s;
        s << "parseBindData(): Failed to extract bind data for 'Bind#"
          << thisBind->bindId() << '\'' << endl;
        cerr  << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parseBindData(): Exit." << endl;
        }

        return false;
    }

    // Looks like a good parse.
    if (mOptions->verbose()) {
        *mDbg << "parseBindData(): Exit." << endl;
    }

    return true;
}


/** @brief Parses a vector of lines relating to a single bind variable to extract the value etc.
 *
 * Parses a vector of lines, read in from the trace file,  which relate to a single
 * bind variable. The value for this current execution's bind variable is extracted and will
 * be used as a substitute for the variable name in the report file when we hit the EXEC for
 * this cursor.
 *
 * In normal use, there should be a value. However, if a bind is used more than once, then
 * the second and subsequent uses will have no bind details at all, only a single line of
 * text stating "  No oacdef for this bind" in which case we need to look up the other binds
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
 *        123 = A buffer. Usually seen in DBMS_OUTPUT.GET_LINES(:LINES, :NUM_LINES). The :LINES bind will be type 123.
 * Avl = Average length. Sometimes means the number of characters in the ASCII representation of the bind's value.
 * Value = The bind's value. May be missing, or "invalid".
 * Mxl = Maximum length, but is not reliable. It's the internal format's maximum length.
 *
 * Returns true if all ok. False otherwise.
 */
bool tmTraceFile::extractBindData(const vector<string> *bindStuff, tmBind *thisBind) {

    if (mOptions->verbose()) {
        *mDbg << "extractBindData(" << mLineNumber << "): Entry." << endl
              << "extractBindData(): Extracting data for Bind #"
              << thisBind->bindId() << '.' << endl;
    }

    // Storage for the data we are extracting from the vector.
    bool noOacdefFound = false;
    unsigned dataType = 0;
    unsigned averageLength = 0;
    string value = "";
    vector<string>::const_iterator valueStartsHere = bindStuff->end();

    // Flags.
    unsigned oacdtyPos = 0;
    unsigned avlPos = 0;
    unsigned noOacdefPos = 0;
    unsigned valuePos = 0;


    // Parse the vector for the data we want.
    for (vector<string>::const_iterator i=bindStuff->begin();
        i != bindStuff->end();
        i++)
    {
        if (mOptions->verbose()) {
           *mDbg << "extractBindData(): Scanning: [" << *i << ']' << endl;
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

            // For now, only...
            noOacdefFound = true;
            value = "Some Other Bind";

            // Nothing more to do.
            break;
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

    // If averageLength is zero, or, we didn't find a "value="
    // then this is most likely an OUT parameter for PL/SQL.
    // Just use the name of the bind as the value.
    if (valuePos == string::npos ||
        averageLength == 0) {
            thisBind->setBindValue(thisBind->bindName());

            if (mOptions->verbose()) {
                *mDbg << "parseBindData(): Suspected OUT PL/SQL parameter found." << endl
                      << "parseBindData(): Exit." << endl;
            }

            return true;

        }

    // We have a good bind, find the value.
    if (!extractBindValue(valueStartsHere, thisBind)) {
        // Damn!
        stringstream s;
        s << "parseBindData(): Failed to extract bind value for 'Bind#"
          << thisBind->bindId() << '\'' << endl;
        cerr  << s.str();

        if (mOptions->verbose()) {
            *mDbg << s.str()
                  << "parseBindData(): Exit." << endl;
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
      *mDbg << "extractNumber(" << mLineNumber << "): Entry." << endl;
   }

   try {
       // Stoul() extracts a number from a string. It stops after the first non numeric character.
       // "E" or "e" might be a problem if we hit one though!
       result = stoul(i->substr(equalPos + 1), NULL, 10);
   } catch (exception &e) {
       stringstream s;
       s << "extractNumber(): Exception " << e.what() << endl
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



/** @brief Extract a binds actual value as a string.
 *
 * @param bindStuff const vector<string>*. Vector of bind data lines.
 * @param i vector<string>::const_iterator. Pointer to the value line in the vector.
 * @param thisBind tmBind*. The bind in question.
 * @return bool. True means all ok. False means problems.
 *
 * The bind data types are:
 *
 *   1 = VARCHAR2 or NVARCHAR2. VARCHAR2 values are in double quotes, NVARCHAR2 values are hex dumps.
 *   2 = NUMBER. Beware if the value is ### which I think is used for an OUT parameter.
 *  11 = ROWID.
 *  23 = RAW. A string of Hex digits.
 *  25 = Unhandled data type.
 *  29 = Unhandled data type.
 *  96 = NCHAR. Dumped as Hex digits.
 * 123 = A buffer. Usually seen in DBMS_OUTPUT.GET_LINES(:LINES, :NUM_LINES). The :LINES bind will be type 123.
 */
bool tmTraceFile::extractBindValue(vector<string>::const_iterator i, tmBind *thisBind) {

   if (mOptions->verbose()) {
      *mDbg << "extractBindValue(" << mLineNumber << "): Entry." << endl;
   }

   unsigned equalPos = i->find("=");
   unsigned quotePos = i->find("\"");

   // We need the data type.
   unsigned dataType = thisBind->bindType();
   switch (dataType) {
        case 1: // VARCHAR2 or NVARCHAR2.
            if ((equalPos + 1) == quotePos) {
                // This is a VARCHAR2.
                string thisValue = i->substr(quotePos);
                thisValue.at(0) = '\'';
                thisValue.at(thisValue.length() - 1) = '\'';
                thisBind->setBindValue(thisValue);
                break;
            } else {
                // This is an NVARCHAR2, extract the hex data instead.
            }
            break;
        case 2: // NUMBER.
            // Number data is on the same text line as the 'value=' text.
            // If the "number" is "###" then we have an OUT parameter in PL/SQL.
            // So, just replace the value with the bind name.
            thisBind->setBindValue(i->substr(equalPos + 1));
            if (thisBind->bindValue() == "###") {
                thisBind->setBindValue(thisBind->bindName());
            }
            break;
        case 11: // ROWID.
            thisBind->setBindValue(i->substr(equalPos + 1));
            break;
        case 23: // RAW.
            break;
        case 25: // UNHANDLED DATA TYPE. (Drop in below).
        case 29: // UNHANDLED DATA TYPE.
            thisBind->setBindValue(i->substr(equalPos + 1));
            break;
        case 96: // NCHAR.
            thisBind->setBindValue(i->substr(equalPos + 1));
            break;
        case 123: // A Data Buffer.valueStartsHere
            break;
        default: // I have no idea what you are!
            break;
   }


   // Looks like a good parse.
   if (mOptions->verbose()) {
      *mDbg << "extractBindValue(): Result = " << thisBind->bindValue() << endl
            << "extractBindValue(): Exit." << endl;
   }

    return true;
}
