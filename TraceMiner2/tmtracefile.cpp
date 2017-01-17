#include "tmtracefile.h"

/** @brief Default constructor.
  *
  * Constructs a new tmTraceFile class without knowing which actual trace file
  * is to be used. Must call SetTraceFile() before attempting to parse anything.
  */
tmTraceFile::tmTraceFile()
{
    mTraceFileName = "";
}

/** @brief Constructor with known trace file name.
  *
  * Constructs a new tmTraceFile class, knowing the name of the trace file to
  * be used.
  */
tmTraceFile::tmTraceFile(string TraceFileName)
{
    mTraceFileName = TraceFileName;
}

/** @brief Destructor.
  *
  * Nothing to see here. Cleans up as required.
  */
tmTraceFile::~tmTraceFile()
{
    // Destructor.
    // Anything to do?
}


/** @brief Parses a trace file.
  *
  * If a trace file is known, the Parse() function will parse it and report
  * the various binds etc used in the EXEC lines found. Only DEP=) EXECs are
  * considered
  */
void tmTraceFile::Parse()
{
    // Process a trace file.

}

