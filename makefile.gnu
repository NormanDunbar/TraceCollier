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
        TraceMiner2/utilities.cpp

OBJECTS=TraceMiner2/TraceMiner2.o \
        TraceMiner2/tmoptions.o \
        TraceMiner2/tmbind.o \
        TraceMiner2/tmcursor.o \
        TraceMiner2/tmtracefile.o \
        TraceMiner2/utilities.o

all:	traceminer2 $(BIN)

traceminer2:	$(OBJECTS) $(BIN)
	$(CPP) -o $(TARGET) $(OBJECTS)
	

$(BIN):
	mkdir $(BIN)

clean: 
	$(RM) $(OBJECTS)

distclean: clean
	$(RM) -r $(BIN)


