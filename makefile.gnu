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
TARGET=$(BIN)/TraceMiner2
RM=rm
BIN=./bin

SOURCES=TraceMiner2/TraceMiner2.cpp \
        TraceMiner2/tmoptions.cpp \
        TraceMiner2/tmbind.cpp \
        TraceMiner2/tmcursor.cpp \
        TraceMiner2/tmtracefile.cpp \
        TraceMiner2/utilities.cpp \
        TraceMiner2/parseExec.cpp \
        TraceMiner2/parseParsing.cpp \
        TraceMiner2/parseParse.cpp \
        TraceMiner2/parseBinds.cpp \
        TraceMiner2/parseParseError.cpp \
        TraceMiner2/parseXctend.cpp \
        TraceMiner2/parseError.cpp

OBJECTS=$(SOURCES:.cpp=.o)

all:	traceminer2 $(BIN)

traceminer2:	$(OBJECTS) $(BIN)
	$(CPP) -o $(TARGET) $(OBJECTS)


$(BIN):
	mkdir $(BIN)

clean:
	$(RM) $(OBJECTS)

distclean: clean
	$(RM) -r $(BIN)


