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

#include "utilities.h"
#include "css.h"

extern string favIcon;

// NOTE: None of these are case insensitive, it's up to the caller to
//       ensure correct case is used. Just a thought!

/** @file utilities.cpp
 * @brief Implementation file for the various utilities in TraceMiner2.
 */


/** @brief Returns the file's extension, without the dot.
 *
 * @param fullPath const std::string&. The full path from which to extract the extension.
 * @return std::string.
 *
 * Scans the supplied full path for a filename and returns the (usually) three character extension part only.
 * The dot prefix for the extension, is not returned.
 */
string fileExtension(const string &fullPath) {

    if (fullPath.empty())
        return "";

    size_t howBig = fullPath.length();

    string::size_type dot = fullPath.rfind('.', howBig);
    if (dot != string::npos) {
        // We have a dot, so we have an extension.
        return(fullPath.substr(dot + 1, howBig - dot));
    }

    // No dot, no extension.
    return "";
}


// Internal use only. Splits a file into a path and full file name, with extension.
// Works on Windows or Unix.
static string splitFile(const string &fullPath, int whichPart) {
    // Do we have a directory separator?
    size_t found = fullPath.find_last_of("/\\");

    // Return path or filename as requested.
    if (whichPart == returnFilePath)
        // If we have a separartor, no worries,
        // if not, return current directory.
    if (found != string::npos) {
        return fullPath.substr(0, found);
    } else {
        return "." + directorySeparator;
    } else {
        return fullPath.substr(found + 1);
    }
}


/** @brief Return the file's name, including the extension.
 *
 * @param fullPath const std::string&. The full path from which to extract the terminal filename and extension.
 * @return std::string
 *
 * Scans the supplied full path for a filename and extension and returns them.
 */
string fileName(const string &fullPath) {

    return splitFile(fullPath, returnFileName);
}


/** @brief Return the file's path.
 *
 * @param fullPath const std::string&. The full path from which to extract the path.
 * @return std::string
 *
 * Scans the supplied full path and returns everything prior to the terminal filename.
 */
string filePath(const string &fullPath) {

    return splitFile(fullPath, returnFilePath);
}


/** @brief Return the file's name, not including the extension, or the dot.
 *
 * @param fullPath const std::string&. The full path from which to extract the filename but not the extension.
 * @return std::string
 *
 * Scans the supplied full path and returns the terminal filename without any extension.
 */
string fileNameOnly(const string &fullPath) {

    string result = splitFile(fullPath, returnFileName);
    string extension = fileExtension(result);

    // No extension? Nothing more to do.
    if (extension.empty())
        return result;

    size_t dot = result.rfind(".");
    if (dot != string::npos) {
        result = result.substr(0, dot);
    }

    return result;
}


/** @brief Strips off the existing extension, and adds on the required replacement.
 *
 * @param fullPath const string&. The full path to the original filename.
 * @param newExtension const string&. The desired new file extension.
 * @return string
 *
 * Returns the input filename with the old extension stripped off and a new one added.
 */
string replaceFileExtension(const string &fullPath, const string &newExtension) {

   size_t dot = fullPath.rfind('.', fullPath.length());
   string result;

   if (dot != string::npos) {
      result = string(fullPath).replace(dot + 1, newExtension.length(), newExtension);
   }

    return result;
}


/** @brief Determines if a given file name exists on the system.
 *
 * @param fullPath const string&. The full path to the original filename.
 * @return bool. Returns true if the file exists, false otherwise.
 */
bool fileExists(const string &fullPath) {

    ifstream *testFile = new ifstream(fullPath);
    if (testFile->good()) {
        // File exists. Hooray!
        testFile->close();
        delete testFile;
        return true;
    } else {
        // File is missing.
        return false;
    }
}


/** @brief Creates a new 'TraceMiner2.css' file, in the same folder as the trace file.
 *
 * @param fullPath const string&. The full path of the CSS filename.
 * @return bool. Returns true if the file was created, false otherwise.
 */
bool createCSSFile(const string &fullPath) {

    ofstream *oCss = new ofstream(fullPath);
    if (oCss->good()) {
        *oCss << cssText << endl;
        oCss->close();
        delete oCss;
        cout << "TraceMiner2: CSS file [" << fullPath << "] created ok." << endl;
        return true;
    } else {
        cerr << "TraceMiner2: CSS file [" << fullPath << "] failed to create." << endl;
        return false;
    }
}


/** @brief Creates a new 'favicon.ico' file, in the same folder as the trace file.
 *
 * @param fullPath const string&. The full path of the favicon.ico file.
 * @return bool. Returns true if the file was created, false otherwise.
 */
bool createFaviconFile(const string &fullPath) {

    ofstream *oFav = new ofstream(fullPath, std::ofstream::out|std::ofstream::binary);
    if (oFav->good()) {
        // The string favIcon is an ASCII representation of the hex codes
        // for the binary data that makes up the favicon.ico file. Pull
        // out two characters at a time, and write them as a single byte
        // (binary that is) to the file.
        while (!favIcon.empty()) {
            string byte = favIcon.substr(0, 2);
            favIcon = favIcon.substr(2);
            char binaryByte = stoul(byte, NULL, 16);
            *oFav << binaryByte;
        }
        oFav->close();
        delete oFav;
        cout << "TraceMiner2: Icon file [" << fullPath << "] created ok." << endl;
        return true;
    } else {
        cerr << "TraceMiner2: Icon file [" << fullPath << "] failed to create." << endl;
        return false;
    }
}


/** @brief Extracts a cursor ID from a string.
 *
 * @param thisLine const string&. The trace line containing the cursor id.
 * @param ok bool*. Success or failure indicator. True is good.
 * @return string. The extracted cursor ID. Empty for not found.
 *
 */
string getCursor(const string &thisLine, bool *ok) {

    string::size_type pos = thisLine.find("#");

    // Everything EXCEPT the PARSING IN CURSOR line has a colon
    // at the end of the cursor id. PARSING IN has a space.
    string::size_type colon = thisLine.find(":", pos);
    if (colon == string::npos) {
        colon = thisLine.find(" ", pos);
    }

    *ok = true;

    // Extract the cursor ID, including the #.
    if (pos != string::npos) {
         return thisLine.substr(pos, colon - pos);
    } else {
        *ok = false;
        return "";
    }
}

/** @brief Extracts numeric text from a string.
 *
 * @param thisLine const string&. The trace line containing the numeric data.
 * @param lookFor const string&. The text immediately prior to the numeric characters.
 * @param ok bool*. Indicator to the success or failure of the extraction. True is good.
 * @return unsigned. The extracted number. Zero if nothing extracted.
 *
 */
unsigned getDigits(const string &thisLine, const string &lookFor, bool *ok) {

    *ok = true;     // Assume all ok.

    unsigned pos = thisLine.find(lookFor);

    if (pos != string::npos) {
        try {
            pos += lookFor.length();
            return stoul(thisLine.substr(pos), NULL, 10);
        } catch (...) {
            ;   //NULL statement. Drops in below.
        }
    }

    // It didn't work.
    *ok = false;
    return 0;
}


/** @brief Enter with a string and the pointer to a colon in that string. Extracts a
 *         bind variable's name, including the colon, extracted from the string.
 *
 * @param thisSQL const string&. The text of the SQL Statement.
 * @param colonPos const string::size_type&. Where to start extracting.
 * @param bindName string&. Somewhere to receive the extracted bind name.
 * @return bool. returns true if all ok, false otherwise.
 *
 */
bool extractBindName(const string &thisSQL, const string::size_type &colonPos, string &bindName) {

    bindName = "";

    // We must be looking at a colon.
    if (thisSQL.at(colonPos) != ':') {
        cerr << "extractBindName(): Not looking at a colon. Cannot extract bind name." << endl;
        return false;
    }

    // If we have a quoted name, scan for the close quote.
    if (thisSQL.at(colonPos + 1) == '"') {
        // We have a shortcut!
        string::size_type closeQuote = thisSQL.find('"', colonPos + 2);
        if (closeQuote != string::npos) {
            bindName = thisSQL.substr(colonPos, closeQuote - colonPos + 1);
            return true;
        } else {
            cerr << "extractBindName(): Malformed SQL. Missing \" in [" << thisSQL << "]." << endl;
            return false;
        }
    }

    // No shortcut, do it the hard way.
    // Stash the colon first.
    unsigned pos = colonPos;
    bindName.push_back(thisSQL.at(pos++));
    char cc = thisSQL.at(pos);

    // Lop around, until we find something  that's not allowed in a bind name.
    while (cc != ' ' &&
           cc != ':' &&
           cc != ',' &&
           cc != ')' &&
           cc != ';')
    {
        bindName.push_back(cc);
        cc = thisSQL.at(++pos);
    }

    return true;
}
