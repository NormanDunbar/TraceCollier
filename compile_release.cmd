rem @echo off
rem Compile all RELEASE versions of the TraceCollier utility.
rem To rebuild all, use rebuild_release.cmd instead.

rem Where is CodeBlocks to be found?
set CB=e:\CodeBlocks\codeblocks.exe

rem What is the project file called?
set CBP=e:\SourceCode\TraceCollier\TraceCollier.cbp

rem What will the logfile be called?
set LOG=e:\SourceCode\TraceCollier\TraceCollier.release.log

rem Build all release targets
echo Building 32bit Release .... > %LOG%
echo =========================== >> %LOG%
echo. >> %LOG%
%CB% --build --target="32bit Release" %CBP% >>%LOG%

echo. >> %LOG%
echo Building 64bit Release .... >> %LOG%
echo =========================== >> %LOG%
echo. >> %LOG%
%CB% --build --target="64bit Release" %CBP% >>%LOG%

echo. >> %LOG%
echo Release Build completed. See %LOG% for details.
