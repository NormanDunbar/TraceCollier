@echo off
rem Rebuild all DEBUG versions of the TraceCollier utility.
rem To compile all, use compile_release.cmd instead.

rem Where is CodeBlocks to be found?
set CB=e:\CodeBlocks\codeblocks.exe

rem What is the project file called?
set CBP=e:\SourceCode\TraceCollier\TraceCollier.cbp

rem What will the logfile be called?
set LOG=e:\SourceCode\TraceCollier\TraceCollier.debug.log

rem Rebuild all release targets
echo Rebuilding 32bit Debug .... > %LOG%
echo =========================== >> %LOG%
echo. >> %LOG%
%CB% --rebuild --target="32bit Debug" %CBP% >>%LOG%

echo. >> %LOG%
echo Rebuilding 64bit Debug .... >> %LOG%
echo =========================== >> %LOG%
echo. >> %LOG%
%CB% --rebuild --target="64bit Debug" %CBP% >>%LOG%

echo. >> %LOG%
echo Debug Rebuild completed. See %LOG% for details.
