#!/bin/bash

# Rebuild all RELEASE versions of the TraceCollier utility.
# To compile all, use compile_release.sh instead.

# Where is CodeBlocks to be found?
CB=`which codeblocks`

# What is the project file called?
CBP=TraceCollier.Linux.cbp

# What will the logfile be called?
LOG=TraceCollier.release.log

# Rebuild all release targets
echo Rebuilding 32bit Release .... > ${LOG}
echo ============================= >> ${LOG}
echo >> ${LOG}
${CB} --rebuild --target="32bit Release" ${CBP} >>${LOG}

echo >> ${LOG}
echo Rebuilding 64bit Release .... >> ${LOG}
echo ============================= >> ${LOG}
echo >> ${LOG}
${CB} --rebuild --target="64bit Release" ${CBP} >>${LOG}

echo >> ${LOG}
echo Release Rebuild completed. See ${LOG} for details.
