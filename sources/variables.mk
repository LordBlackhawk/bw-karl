SOURCEPATH   = ./
INCLUDEPATH  = ../
BASEOUTPATH  = $(PROJECTPATH)out/
OBJECTPATH   = $(BASEOUTPATH)$(MODULENAME)/
BWAPIPATH    = $(PROJECTPATH)includes/
BOOSTPATH    = $(PROJECTPATH)includes/boost_1_46_1
LIBPATH      = $(PROJECTPATH)lib/

BOOST_LIBS   = -lboost_thread-mgw47-mt-1_46_1 -lboost_program_options-mgw47-mt-1_46_1
ifeq ($(NEEDS_TEST_LIBS), yes)
    BOOST_LIBS +=  -lboost_unit_test_framework-mgw47-mt-1_46_1 -lboost_test_exec_monitor-mgw47-mt-1_46_1
endif

CC           = gcc
CCFLAGS      = -g -Wall -Wextra -O1 -fno-strict-aliasing
ASFLAGS      = -g -fexceptions -xassembler-with-cpp

CXX          = g++
CXXINCLUDES  = -I$(BWAPIPATH) -I$(INCLUDEPATH)
CXXFLAGS     = -g -Wall -Wextra -O3 -fno-omit-frame-pointer $(CXXINCLUDES)
CXXLIBS      = -L$(LIBPATH) $(BOOST_LIBS) -lBWTA -lBWAPI -lCGAL -lmpfr -lgmp -static-libgcc -static-libstdc++

SOURCES      = $(wildcard $(SOURCEPATH)*.cpp)
CSOURCES     = $(wildcard $(SOURCEPATH)*.cc)
SSOURCES     = $(wildcard $(SOURCEPATH)*.s)
OBJECTS      = $(addprefix $(OBJECTPATH), $(notdir $(SOURCES:.cpp=.o) $(CSOURCES:.cc=.o) $(SSOURCES:.s=.o))) 
DEPS         = $(addprefix $(OBJECTPATH), $(notdir $(SOURCES:.cpp=.d) $(CSOURCES:.cc=.d)))

MEMCHECK     = ../DrMemory-Windows-1.5.0-5/bin/drmemory.exe
