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

#ifndef TMOPTIONS_H
#define TMOPTIONS_H

/** @file tmoptions.h
 * @brief Header file for the tmOptions object.
 */

#include <string>
#include <iostream>

using std::string;
using std::cerr;
using std::endl;

/** @brief A class representing the command line options for TraceMiner2.
 */
class tmOptions
{
    public:
        tmOptions();
        virtual ~tmOptions();

        // Getters.
        bool verbose() { return mVerbose; }             /**< Returns verbose mode flag. */
        bool html() { return mHtml; }                   /**< Returns HTML mode flag. */
        bool help() { return mHelp; }                   /**< Returns help mode flag. */

        string traceFile() { return mTraceFile; }       /**< Returns trace file name. */
        string reportFile() { return mReportFile; }     /**< Returns report file name. */
        string debugFile() { return mDebugFile; }       /**< Returns debug information file name. */

        string htmlExtension() { return mHtmlExtension; }       /**< Returns HTML report file extension. */
        string reportExtension() { return mReportExtension; }   /**< Returns TEXT report file extension. */
        string debugExtension() { return mDebugExtension; }     /**< Returns debug information file extension. */
        string cssFileName() { return mCssFileName; }           /**< Returns default CSS filename. */

        // Setters.
        void setVerbose(bool verbose) { mVerbose = verbose; }   /**< Sets the verbose flag, if required. */

        void usage();                               /**< Display usage and force an exit. */
        bool parseArgs(int argc, char *argv[]);     /**< Parses command line arguments and sets various flags etc. */

    protected:

    private:
        bool mVerbose;                      /**< Are we running in verbose mode? */
        bool mHtml;                         /**< Are we reporting in HTML? */
        bool mHelp;                         /**< Did the user request help? */
        string mTraceFile;                  /**< Name of the trace file being parsed. */
        string mReportFile;                 /**< Name of the report file. */
        string mDebugFile;                  /**< Name of the debug information file. */
        string mCssFileName;                /**< Full path & name of the actual CSS file. */

        string mReportExtension = "log";    /**< Default extension for the text report file. */
        string mHtmlExtension = "html";     /**< Default extension for the HTML report file. */
        string mDebugExtension = "dbg";     /**< Default extension for the debug information file. */
};

#endif // TMOPTIONS_H
