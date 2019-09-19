#!/bin/bash

# Compile all DEBUG versions of the TraceMiner2 utility.
# To rebuild all, use rebuild_release.sh instead.

# Where is CodeBlocks to be found?
CB=`which codeblocks`

# What is the project file called?
CBP=TraceMiner2.Linux.cbp

# What will the logfile be called?
LOG=TraceMiner2.debug.log

# Build all release targets
echo Building 32bit Debug .... > ${LOG}
echo ========================= >> ${LOG}
echo >> ${LOG}
${CB} --build --target="32bit Debug" ${CBP} >>${LOG}

echo >> ${LOG}
echo Building 64bit Debug .... >> ${LOG}
echo ========================= >> ${LOG}
echo >> ${LOG}
${CB} --build --target="64bit Debug" ${CBP} >>${LOG}

echo >> ${LOG}
echo Debug Build completed. See ${LOG} for details.
