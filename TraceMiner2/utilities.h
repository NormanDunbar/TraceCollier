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

#ifndef UTILITIES_H
#define UTILITIES_H

/** @file utilities.h
 * @brief Header file for the utility functions used in TraceMiner2.
 */

#include <string>
#include <fstream>
#include <iostream>

using std::string;
using std::ifstream;
using std::ofstream;
using std::endl;
using std::cout;
using std::cerr;

// Try to figure out the directory separator.
#if defined(_WIN32) || defined(_WIN64)
    const string directorySeparator = "\\";
#else
    const string directorySeparator = "/";
#endif // _WINDOWS_

// Some constants for internal use.
const int returnFilePath = 0;
const int returnFileName = 1;
const string eol = "\n";

string fileExtension(const string &fullPath);   /**< Return the file extension for a given file name. */
string fileName(const string &fullPath);        /**< Return the file name part, including extension, for a full path. */
string filePath(const string &fullPath);        /**< Return the file path for a given file name. */
string fileNameOnly(const string &fullPath);    /**< Return the file's name, not including the extension, or the dot.  */
string replaceFileExtension(const string &fullPath, const string &newExtension);    /**< Return a string with the file name extension changed. */
bool fileExists(const string &fullPath);        /**< Returns true if a given file path exists, false if not. */
bool createCSSFile(const string &fullPath);     /**< Creates the default CSS file. Returns true if ok, False otherwise. */
bool createFaviconFile(const string &fullPath); /**< Creates the favicon.ico file. Returns true if ok, False otherwise. */

string getCursor(const string &thisLine, bool *ok); /**< Extract a cursor id from a trace line. */
unsigned getDigits(const string &thisLine, const string &lookFor, bool *ok); /**< Extract a number from a trace line. */
bool extractBindName(const string &thisSQL, const string::size_type &colonPos, string &bindName);   /**< Extract the bind variable name from a SQL Statement. */

#endif // UTILITIES_H
