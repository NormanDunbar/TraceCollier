# A simple makefile to be used on Linux, with GNU make.
# To see what it will do, without doing it, run:
# make -n -f makefile.gnu
#
# Norman Dunbar.
# January 2017.
#
# PS. It's "noisy" anyway. So you'll see what's what on the screen.
#

CPP=g++
CPPFLAGS=-std=c++11 
TARGET=$(BIN)/TraceCollier
RM=rm
BIN=./bin
STRIP=strip

SOURCES=TraceCollier/TraceCollier.cpp \
        TraceCollier/tmoptions.cpp \
        TraceCollier/tmbind.cpp \
        TraceCollier/tmcursor.cpp \
        TraceCollier/tmtracefile.cpp \
        TraceCollier/utilities.cpp \
        TraceCollier/parseExec.cpp \
        TraceCollier/parseParsing.cpp \
        TraceCollier/parseParse.cpp \
        TraceCollier/parseBinds.cpp \
        TraceCollier/parseClose.cpp \
        TraceCollier/parseStat.cpp \
        TraceCollier/parseParseError.cpp \
        TraceCollier/parseXctend.cpp \
        TraceCollier/parseDeadlock.cpp \
        TraceCollier/parseError.cpp

OBJECTS=$(SOURCES:.cpp=.o)

all:	TraceCollier $(BIN)

TraceCollier:	$(OBJECTS) $(BIN)
	$(CPP) -o $(TARGET) $(OBJECTS)
	$(STRIP) $(TARGET)


$(BIN):
	mkdir $(BIN)

clean:
	$(RM) $(OBJECTS)

distclean: clean
	$(RM) -r $(BIN)


