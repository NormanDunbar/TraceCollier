#ifndef UTILITIES_H
#define UTILITIES_H

/** @file utilities.h
 * @brief Header file for the utility functions used in TraceMiner2.
 */

#include <string>

using std::string;

// Try to figure out the directory separator.
#if defined(_WIN32) || defined(_WIN64)
    const string directorySeparator = "\\";
#else
    const string directorySeparator = "/";
#endif // _WINDOWS_

// Some constants for internal use.
const int returnFilePath = 0;
const int returnFileName = 1;

string fileExtension(const string &fullPath);   /**< Return the file extension for a given file name. */
string fileName(const string &fullPath);        /**< Return the file name part, including extension, for a full path. */
string filePath(const string &fullPath);        /**< Return the file path for a given file name. */
string fileNameOnly(const string &fullPath);    /**< Return the file's name, not including the extension, or the dot.  */
string replaceFileExtension(const string &fullPath, const string &newExtension);    /**< Return a string with the file name extension changed. */

#endif // UTILITIES_H
