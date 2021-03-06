/** @mainpage Trace Collier
 *
 *  @author Norman Dunbar
 *  @copyright MIT Licence
 *
 * @tableofcontents
 *
 *  @section Introduction
 *
 * TraceCollier is a new improved version of the old TraceMiner utility. It has
 * been completely rewritten in C++ rather than the old style vanilla C. This has made
 * things surprisingly easier to do in the new versions and has got around a whole pile
 * of crud and hacks in the previous version. There's no manual configuration required
 * before compiling for example.
 *
 * @section sec-free-source-code Free Source Code
 *
 * The source code for TraceCollier lives in GitHub at
 * https://github.com/NormanDunbar/TraceCollier/.
 *
 * Feel free to clone the repository and make any desired changes you wish.
 *
 * You may, if you just want to compile and use the code, download the source as a
 * zip file from https://github.com/NormanDunbar/TraceCollier/archive/master.zip.
 *
 * @section sec-compiling Compiling
 *
 * The source should be downloaded. Then simply compile all the *.cpp files using a C++
 * compiler that knows about the Standard Template Library (aka STL). This is the only
 * "weird" thing used by TraceCollier. No other special libraries such as Boost, for
 * example, are required.
 *
 * @subsection sub-sec-with-codeblocks With CodeBlocks
 *
 * There's a CodeBlocks project file, `TraceCollier.cbp`, located in the main source folder.
 * You may need to change some paths etc to use it. In my own setup, I have configured the
 * free Borland/Embarcadero C++ compiler, version 10.1, to be used. Get yours at
 * https://www.embarcadero.com/free-tools/ccompiler/start-for-free. CodeBlocks itself is
 * available at https://www.codeblocks.org.
 *
 * Once compiled, look here for your executable:
 *
 * @code
 * TraceCollier-master\TraceCollier\bin\release
 * @endcode
 *
 * @subsection sub-sec-with-borland With Borland/Embarcadero C++ 10.1
 *
 * Unzip the download zip file, then:
 *
 * @code
 * cd TraceCollier-master\TraceCollier
 * mkdir bin
 * bcc32c -o bin\TraceCollier.exe *.cpp
 * @endcode
 *
 * Once compiled, look your executable will be:
 *
 * @code
 * TraceCollier-master\TraceCollier\bin\TraceCollier.exe
 * @endcode
 *
 * Get a free copy of the compiler at https://www.embarcadero.com/free-tools/ccompiler/start-for-free.
 *
 * @subsection sub-sec-with-gpp With G++
 *
 * You can use the supplied Makefile.gnu, or just compile all the *.cpp files, as follows:
 *
 * @code
 * cd TraceCollier-master/TraceCollier
 * mkdir bin
 * g++ -o bin/TraceCollier *.cpp
 * @endcode
 *
 * The executable will be wherever you pointed it to be created with the '-o' option.
 *
 * @subsection sec-makefile With a Makefile
 *
 * @code
 * cd TraceCollier-master
 * make -f makefile.gnu
 * @endcode
 *
 * Once compiled, look here for your executable:
 *
 * @code
 * TraceCollier-master/TraceCollier/bin
 * @endcode
 *
 * @section sec-execution Execution
 *
 * There is one mandatory parameter required, the trace file name. This must have the extension
 * ".trc" as generated by Oracle. The report file will have the ".trc" changed to ".txt" or
 * ".html" depending on whether you have requested a plain text or HTML report format.
 *
 * From TraceCollier version 1.10, trace files can be either stand-alone, or those created by
 * the 'trcsess' utility which combined numerous trace files for a single session into one, large, one.
 *
 * The report file will be created in the same location as the trace file and defaults to HTML format. This
 * is simply because the textual format is pretty dire with wide lines in many cases.
 *
 * If you choose to run in verbose mode, and you probably shouldn't, the output file for that
 * detail will be the same as the trace file name, but with the ".trc" extension replaced by
 * ".dbg". Once again, the output file will be in the same location as the trace file.
 *
 * The optional parameters are as follows:
 *
 * @li --verbose or -v - indicates that you wish to generate lots of output for debugging purposes.
 * @li --quiet or -q - indicates that you do not wish to see "Cursor: #cccc created at line: nnnn" messages.
 * on the screen while parsing is taking place. Any ERRORs or PARSE ERRORS will still be displayed.
 * @li --help -h or -? - indicates that you want help. The program will exit after displaying the
 * usage details.
 * @li --text or -t - indicates that you wish to have the report formatted in plain text as opposed to
 * HTML text. Good luck with that option! ;-)
 * @li --depth=n or -d=n - indicates the maximum depth of recursive cursors that you wish to report on
 * which is useful when a PL/SQL calls is at depth=0, you wouold be interested in depth=1, for example.
 * @li --pagesize=nn or -p=nn - indicates how many EXEC statements you wish to display in each HTML table
 * in the report. This meakes scrolling to the headings in long report files a lot easier!
 * @li --feedback=nn or -f=nn - indicates how often you want feedback on progress reading the trace file. Zero
 * disables feedback. The default is every 100,000 lines read. Useful on larger trace files.
 *
 * @section sec-mit-licence MIT Licence
 *
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

/** @file TraceCollier.cpp
 * @brief This is where it all starts. The main() function lives here.
 */


#include "TraceCollier.h"
#include "utilities.h"
#include "favicon.h"

// Version number.
const float version = 1.50;

// Various flags set according to the passed parameters.
tmOptions options;

int main(int argc, char *argv[])
{
    // Sign on.
    cout << "TraceCollier version " << version;
    #ifdef USE_REGEX
        cout << " : with REGEX support.";
    #else
        cout << " : with no REGEX support.";
    #endif // USE_REGEX
    cout << endl << endl;

    // Make sure that cout/cerr gets comma/dot separated thousands etc.
    cout.imbue(locale(cout.getloc(), new ThousandsSeparator<char>(',')));
    cerr.imbue(locale(cerr.getloc(), new ThousandsSeparator<char>(',')));


    // Parse command line args and bale if problems detected.
    bool allOk = options.parseArgs(argc, argv);
    if (!allOk) {
        return 1;
    }

    // Show help and exit requested?
    if (options.help()) {
        return 0;
    }

    if (options.html()) {
        // Create a (new) CSS file, if HTML requested and
        // there isn't one already.
        string cssFile = options.cssFileName();
        if (fileExists(cssFile)) {
            cout << "File exists: " << cssFile << endl;
        } else {
            allOk = createCSSFile(cssFile);
            if (!allOk) {
                return 1;
            }
        }

        // Likewise, a 'favicon.ico' icon file too.
        string favIconFile = filePath(cssFile) + directorySeparator + "favicon.ico";
        if (fileExists(favIconFile)) {
            cout << "File exists: " << favIconFile << endl;
        } else {
            allOk = createFaviconFile(favIconFile);
            if (allOk) {
                cout << "TraceCollier: 'favicon' file [" << favIconFile << "] created ok." << endl;
            } else {
                cout << "TraceCollier: 'favicon' file [" << favIconFile << "] creation failed." << endl;
                return 1;
            }
        }
    }



    // This is it, here is where we hit the big time! :)
    tmTraceFile *traceFile = new tmTraceFile(&options);
    allOk = traceFile->parse(version);

    // All done.
    if (traceFile) {
        delete traceFile;
    }

    return allOk ? 0 : 1;
}
