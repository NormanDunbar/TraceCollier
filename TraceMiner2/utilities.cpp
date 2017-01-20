#include "utilities.h"

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

    size_t dot = fullPath.rfind('.', howBig);
    if (dot != string::npos) {
        return(fullPath.substr(dot + 1, howBig - dot));
    }

    return("");
}


// Internal use only. Splits a file into a path and full file name, with extension.
// Works on Windows or Unix.
static string splitFile(const string &fullPath, int whichPart) {
    size_t found = fullPath.find_last_of("/\\");

    if (whichPart == returnFilePath)
        return fullPath.substr(0, found);
    else
        return fullPath.substr(found + 1);
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

