@echo off
rem Rebuild all RELEASE versions of the TraceMiner2 utility.
rem To compile all, use compile_release.cmd instead.

rem Where is CodeBlocks to be found?
set CB=e:\CodeBlocks\codeblocks.exe

rem What is the project file called?
set CBP=e:\SourceCode\TraceMiner2\TraceMiner2.cbp

rem What will the logfile be called?
set LOG=e:\SourceCode\TraceMiner2\TraceMiner2.release.log

rem Rebuild all release targets
echo Rebuilding 32bit Release .... > %LOG%
echo ============================= >> %LOG%
echo. >> %LOG%
%CB% --rebuild --target="32bit Release" %CBP% >>%LOG%

echo. >> %LOG%
echo Rebuilding 64bit Release .... >> %LOG%
echo ============================= >> %LOG%
echo. >> %LOG%
%CB% --rebuild --target="64bit Release" %CBP% >>%LOG%

echo. >> %LOG%
echo Release Rebuild completed. See %LOG% for details.
