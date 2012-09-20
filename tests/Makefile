OBJECTPATH   = ../out/
SOURCEPATH   = ../karl-2/
BWAPIPATH    = ../bwapi/trunk/bwapi/include
LIBPATH		 = ../

ASSIGNMENT-TEST-SOURCES  = hungarian-algorithm.cpp timer.cpp
ASSIGNMENT-TEST-PSOURCES = $(addprefix $(SOURCEPATH), $(ASSIGNMENT-TEST-SOURCES))

CXX          = g++ -ggdb
CXXINCLUDES  = -I../ -I$(SOURCEPATH) -I$(BWAPIPATH)
CXXFLAGS     = -Wall -Wextra -O3 --std=c++0x $(CXXINCLUDES)
CXXLIBS      = -L$(LIBPATH) -lBWAPI

assignment-test.exe: assignment-test.cpp $(ASSIGNMENT-TEST-PSOURCES)
	$(CXX) $(CXXINCLUDES) $(CXXFLAGS) assignment-test.cpp $(ASSIGNMENT-TEST-PSOURCES) -o assignment-test.exe
	
assignment: assignment-test.exe
	assignment-test.exe
