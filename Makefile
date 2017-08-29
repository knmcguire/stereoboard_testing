#############################################################################
# Makefile for building: Evolutionary Learning
#############################################################################

PROJECT      ?= edgeflow

####### Compiler, tools and options

CC            = gcc
CXX           = g++ 
DEFINES	      = -DCOMPILE_ON_LINUX
CFLAGS        = -pipe -g -Wall -W $(DEFINES) $(shell pkg-config --cflags opencv) -fpermissive
CXXFLAGS      = -pipe -g -Wall -W $(DEFINES) -MMD -std=c++11 -fpermissive
CXXFLAGS      += $(shell pkg-config --cflags opencv)
LINK          = g++
LFLAGS        = $(shell pkg-config --libs opencv)
LIBS          = $(SUBLIBS) -lrt -pthread -lopencv_core -lopencv_highgui -lopencv_imgproc
DEL_FILE      = rm -f

CV_PATH		  = ../drone_vision/cv
INC_PATH	  = -I../../common -I../../stereoboard -I../../stereoboard/drivers/inc -I../../stereoboard/math
CXXFLAGS	  +=-I${CV_PATH} ${INC_PATH}

####### Output directory

OBJECTS_DIR   = ./

####### Files

TARGET  = testing

SOURCES = $(CV_PATH)/image.c $(wildcard ../../stereoboard/math/*.c)

ifeq ($(PROJECT), gate)
	SOURCES += door_detection.cpp \
            ../../stereoboard/gate_detection.c \
            ../../stereoboard/stereo_image.c
            
    DEFINES += -DGATE_DETECTION_GRAPHICS=1 -DGATE_ROTATE=1 -DGATE_SHAPE=6 -DGATE_NSAMPLES=1500
else
    SOURCES += main.cpp \
            ../../stereoboard/edgeflow.c \
            $(CV_PATH)/encoding/jpeg.c
endif

$(info $(SOURCES))

CPP_OBJECTS = $(SOURCES:.cpp=.o)
OBJECTS = $(CPP_OBJECTS:.c=.o)

$(info $(OBJECTS))

first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) -o "$@" "$<"

.c.o:
	$(CC)  -c $(CFLAGS) -o "$@" "$<"

####### Build rules
.PHONY : all clean

all: Makefile $(TARGET)

$(TARGET):  $(OBJECTS)
	$(LINK) $(LIBS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LFLAGS)
	{ test -n "$(DESTDIR)" && DESTDIR="$(DESTDIR)" || DESTDIR=.; } && test $$(gdb --version | sed -e 's,[^0-9]\+\([0-9]\)\.\([0-9]\).*,\1\2,;q') -gt 72 && gdb --nx --batch --quiet -ex 'set confirm off' -ex "save gdb-index $$DESTDIR" -ex quit '$(TARGET)' && test -f $(TARGET).gdb-index && objcopy --add-section '.gdb_index=$(TARGET).gdb-index' --set-section-flags '.gdb_index=readonly' '$(TARGET)' '$(TARGET)' && rm -f $(TARGET).gdb-index || true

debug:
	gdb $(TARGET)
  
clean: 
	$(MAKE) -C ../../stereoboard clean
	$(DEL_FILE) $(wildcard *.o) $(wildcard *.d) $(TARGET)

####### Compile

%.o: %.c 
	$(CXX) -c $(CXXFLAGS) $(DEPFLAGS) -o $@ $<
	
####### Install

install:   FORCE

uninstall:   FORCE

FORCE:

-include $(OBJECTS:.o=.d)
