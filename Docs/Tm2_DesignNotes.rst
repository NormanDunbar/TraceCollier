=============
Trace Miner 2
=============

Abstract
========

Similar to TraceMiner, but in C++. Reads a trace file and extracts the user level (depth = 0) SQL statements, replacing the bind variables in the SQL with the actual bind values at EXEC time.

We ignore pretty much everything except:

- PARSING IN CURSOR - Used to extract the SQL text from the trace file.
- PARSE - Determines when the SQL was most recently parsed. The trace file line number of the parse line, is listed in the report file for every execution.
- BINDS - Used to extract the bind values for a particular cursor, prior to an execution.
- EXEC - Used to write the details to the report file.
- COMMIT/ROLLBACK - As for EXEC.
- CLOSE - Doesn't do much, other than setting a flag in the cursor. Could be used to check if the trace file is uncorrupted. Maybe!
- ERROR - Reported in the report file.
- PARSE ERROR - Reported in the report file.

Application Parameters
======================

The following parameters, to be supplied at run time, have been identified:

Parameters
----------

Trace File Name - **Mandatory**. The Oracle trace file to be parsed.

- -m | --html - output should be in HTML please.
- -v | --verbose - Provide detailed debugging output. Requires a debug file to be present.
- -h | -? | --help - Provide usage details.

Classes Overview
================

The following classes have been identified as being *somewhat* useful:

- tmTraceFile - Trace File class. Owns all cursors. Controls output, parsing etc.
- tmCursor - Cursor class. Created at PARSING IN, closed at CLOSE and re-opened at PARSE.
- tmBind - One bind value. Owned by a parent cursor just prior to EXEC time.
- tmReportBase - base class for the tmTextFile and tmHTMLFile classes.
- tmTextFile - Output file handler, in text format.
- tmHTMLFile - output file handler, in HTML format.
- tmDebugFile - Output file handler, for debugging information in verbose mode.

There *may* be other output formats classes as and when I can think of a good reason to have one!

Classes
=======

tmTraceFile
-----------

Members
~~~~~~~

- mTraceFileName - string - Obvious, really?
- mCursors - map<string, tmCursor> - list of opened cursors in this trace file.


Functions
~~~~~~~~~

- **Constructor(TraceFileName)** - handles opening of the trace file, creates and owns the other classes, parses the trace file.
- **Parse()** - handles the parsing of a trace file.
- **Destructor()** - closes the trace file, destroys all owned classes.


tmCursor
--------

Members
~~~~~~~

- mCursorId - string - the actual cursor id from the PARSING line.
- mSQLSize - int - the size of the SQL string, as per the "len=" detail.
- mSQLText - string - the actual SQL text from the trace file. Binds have been removed and replaced with %s.
- mSQLLineNumber - int - the line where the SQL text is found in the trace file.
- mSQLParseLine - unsigned - the line where this cursor was most recently parsed.


Functions
~~~~~~~~~

- **Constructor()** - 
- **exec()** - Called when a cursor's EXEC line is found. Prints out the EXEC details, with binds replaced by actual values.
- **Destructor()** - 


tmBind
------

Members
~~~~~~~

Functions
~~~~~~~~~

- **Constructor())** - 
- **Destructor()** - 


tmReportBase
------------

Members
~~~~~~~

Functions
~~~~~~~~~

- **Constructor(ReportFileName)** - 
- **Destructor()** - 


tmTextFile
----------

Members
~~~~~~~

Functions
~~~~~~~~~

- **Constructor(TextFileName)** - 
- **Destructor()** - 


tmHTMLFile
----------

Members
~~~~~~~

Functions
~~~~~~~~~

- **Constructor(HTMLFileName))** - 
- **Destructor()** - 


Beware Of ...
=============

Bind Value Missing
------------------

At some point in the trace file, you will come across a bind that has no value. Not only that, the EOL after where the value would be, is missing, and the line holds the following line too. The line looks like this::

    value= Bind#
    
which is a right pain as we need to trap that Bind# part for the following bind.


Data Type 96
------------

The output is in hex. It is usually a zero byte and an ASCII byte, for example, '1234' is::

    0 31 0 32 0 33 0 34
    
If the column is wider than 50 characters, the trace file emits a second line with the hex codes for character 51 onwards. TraceMiner 0.21 ignores the subsequent lines in this case, TraceMiner 2 should do it correctly. The second line (and subsequent?) start immediately in column zero of the trace.


Bind Reuse
----------

An SQL statement can use the same bind more than once::

    insert into table_name values (:1, :2, :1, :2, :3 ...);
    
The second and subsequent use of a bind results in a "no ocadef for this bind" message in the tracefile, as in::

    ...
     Bind#1
        No oacdef for this bind.
    EXEC #4572736112:c=57,e=91,p=0,...
    ...


PL/SQL Assignments
------------------

PL/SQL assignments are of the format::

    vResult := function_call(:1, :fred, 'Wilma', 'BedRock' ...);
    
It is considered bad form to treat the assignment as a bind!