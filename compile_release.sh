#!/bin/bash

# Compile all RELEASE versions of the TraceCollier utility.
# To rebuild all, use rebuild_release.sh instead.

# Where is CodeBlocks to be found?
CB=`which codeblocks`

# What is the project file called?
CBP=TraceCollier.Linux.cbp

# What will the logfile be called?
LOG=TraceCollier.release.log

# Build all release targets
echo Building 32bit Release .... > ${LOG}
echo =========================== >> ${LOG}
echo >> ${LOG}
${CB} --build --target="32bit Release" ${CBP} >>${LOG}

echo >> ${LOG}
echo Building 64bit Release .... >> ${LOG}
echo =========================== >> ${LOG}
echo >> ${LOG}
${CB} --build --target="64bit Release" ${CBP} >>${LOG}

echo >> ${LOG}
echo Release Build completed. See ${LOG} for details.
