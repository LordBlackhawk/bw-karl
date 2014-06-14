SOURCEPATH   = ./
INCLUDEPATH  = ../
OBJECTPATH   = $(PROJECTPATH)out/$(MODULENAME)/
BWAPIPATH    = $(PROJECTPATH)includes/
BOOSTPATH    = $(PROJECTPATH)../boost_1_46_1
LIBPATH      = $(PROJECTPATH)lib/

BOOST_LIBS   = -lboost_thread-mgw47-mt-1_46_1
ifeq ($(NEEDS_TEST_LIBS), yes)
    BOOST_LIBS +=  -lboost_unit_test_framework-mgw47-mt-1_46_1 -lboost_test_exec_monitor-mgw47-mt-1_46_1
endif

CXX          = g++
CXXINCLUDES  = -I$(BWAPIPATH) -I$(BOOSTPATH) -I$(INCLUDEPATH)
CXXFLAGS     = -Wall -Wextra -O3 $(CXXINCLUDES)
CXXLIBS      = -L$(LIBPATH) -L$(BOOSTPATH)/stage/lib $(BOOST_LIBS) -lBWTA -lBWAPI -lCGAL -lmpfr -lgmp -static-libgcc -static-libstdc++

SOURCES      = $(wildcard $(SOURCEPATH)*.cpp)
OBJECTS      = $(addprefix $(OBJECTPATH), $(notdir $(SOURCES:.cpp=.o))) 
DEPS         = $(addprefix $(OBJECTPATH), $(notdir $(SOURCES:.cpp=.d)))

MEMCHECK     = ../DrMemory-Windows-1.5.0-5/bin/drmemory.exe
