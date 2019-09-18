@echo off
rem Compile all DEBUG versions of the TraceMiner2 utility.
rem To rebuild all, use rebuild_release.cmd instead.

rem Where is CodeBlocks to be found?
set CB=e:\CodeBlocks\codeblocks.exe

rem What is the project file called?
set CBP=e:\SourceCode\TraceMiner2\TraceMiner2.cbp

rem What will the logfile be called?
set LOG=e:\SourceCode\TraceMiner2\TraceMiner2.debug.log

rem Build all release targets
echo Building 32bit Debug .... > %LOG%
echo ========================= >> %LOG%
echo. >> %LOG%
%CB% --build --target="32bit Debug" %CBP% >>%LOG%

echo. >> %LOG%
echo Building 64bit Debug .... >> %LOG%
echo ========================= >> %LOG%
echo. >> %LOG%
%CB% --build --target="64bit Debug" %CBP% >>%LOG%

echo. >> %LOG%
echo Debug Build completed. See %LOG% for details.
