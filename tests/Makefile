OBJECTPATH   = ../out/
SOURCEPATH   = ../karl/
BWAPIPATH    = ../includes
LIBPATH		 = ../
EXEPATH		 = ../

ASSIGNMENT-TEST-SOURCES  = hungarian-algorithm.cpp timer.cpp
ASSIGNMENT-TEST-PSOURCES = $(addprefix $(SOURCEPATH), $(ASSIGNMENT-TEST-SOURCES))

CXX          = g++ -ggdb
CXXINCLUDES  = -I../ -I$(SOURCEPATH) -I$(BWAPIPATH)
CXXFLAGS     = -Wall -Wextra -O3 --std=c++0x $(CXXINCLUDES)
CXXLIBS      = -L$(LIBPATH) -lBWAPI

$(EXEPATH)assignment-test.exe: assignment-test.cpp $(ASSIGNMENT-TEST-PSOURCES)
	$(CXX) $(CXXINCLUDES) $(CXXFLAGS) $(CXXLIBS) assignment-test.cpp $(ASSIGNMENT-TEST-PSOURCES) -o $@
	
assignment: $(EXEPATH)assignment-test.exe
	$<

$(EXEPATH)mineral-speed-test.exe: mineral-speed-test.cpp
	$(CXX) $(CXXINCLUDES) $(CXXFLAGS) $(CXXLIBS) mineral-speed-test.cpp -o $@ -lBWAPI

mineral-speed: $(EXEPATH)mineral-speed-test.exe
	$<
	