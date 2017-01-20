#ifndef TMOPTIONS_H
#define TMOPTIONS_H

#include <string>
#include <iostream>

#include "utilities.h"

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
        bool Verbose() { return mVerbose; }             /**< Returns verbose mode flag. */
        bool Html() { return mHtml; }                   /**< Returns HTML mode flag. */
        bool Help() { return mHelp; }                   /**< Returns help mode flag. */

        string traceFile() { return mTraceFile; }       /**< Returns trace file name. */
        string reportFile() { return mReportFile; }     /**< Returns report file name. */
        string debugFile() { return mDebugFile; }       /**< Returns debug information file name. */

        string htmlExtension() { return mHtmlExtension; }       /**< Returns HTML report file extension. */
        string reportExtension() { return mReportExtension; }   /**< Returns TEXT report file extension. */
        string debugExtension() { return mDebugExtension; }     /**< Returns debug information file extension. */

        // There are no Setters.

        void usage();                               /**< Display usage and force an exit. */
        bool ParseArgs(int argc, char *argv[]);     /**< Parses command line arguments and sets various flags etc. */

    protected:

    private:
        bool mVerbose;                      /**< Are we running in verbose mode? */
        bool mHtml;                         /**< Are we reporting in HTML? */
        bool mHelp;                         /**< Did the user request help? */
        string mTraceFile;                  /**< Name of the trace file being parsed. */
        string mReportFile;                 /**< Name of the report file. */
        string mDebugFile;                  /**< Name of the debug information file. */

        string mReportExtension = "log";    /**< Default extension for the text report file. */
        string mHtmlExtension = "html";     /**< Default extension for the HTML report file. */
        string mDebugExtension = "dbg";     /**< Default extension for the debug information file. */
};

#endif // TMOPTIONS_H
