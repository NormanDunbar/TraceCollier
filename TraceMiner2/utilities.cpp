#include "utilities.h"

// NOTE: None of these are case insensitive, it's up to the caller to
//       ensure correct case is used. Just a thought!


// Returns the file's extension, without the dot.
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


// Return the file's name, including the extension.
string fileName(const string &fullPath) {

    return splitFile(fullPath, returnFileName);
}


// Return the file's path only.
string filePath(const string &fullPath) {

    return splitFile(fullPath, returnFilePath);
}


// Return the file's name, not including the extension, or the dot.
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


// Strips off the existing extension, and adds on the required replacement.
string replaceFileExtension(const string &fullPath, const string &newExtension) {

   size_t dot = fullPath.rfind('.', fullPath.length());
   string result;

   if (dot != string::npos) {
      result = string(fullPath).replace(dot + 1, newExtension.length(), newExtension);
   }

    return result;
}

