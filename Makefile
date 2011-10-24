OBJECTPATH  = out/
BWAPIPATH   = ../bwapi/trunk/bwapi/include
BWTAPATH    = ../bwta/include

CXX         = g++
CXXFLAGS    = -Wall -Wextra -O2 --std=c++0x -I$(BWAPIPATH) -I$(BWTAPATH)
CXXLIBS     = -L. -lBWAPI -lBWTA

# CSOURCES    = $(wildcard $(CLIENTPATH)*.cpp)
# LSOURCES    = $(wildcard $(LIBPATH)*.cpp)
# SSOURCES    = $(wildcard $(SHAREDPATH)*.cpp)
# USOURCES    = $(wildcard $(UTILPATH)Util/*.cpp)
# SOURCES     = $(CSOURCES) $(LSOURCES) $(SSOURCES) $(USOURCES) $(LIBPATH)../UnitCommand.cpp
# OBJECTS     = $(addprefix $(OBJECTPATH), $(notdir $(SOURCES:.cpp=.o)))

all: ExampleAIClient.exe

ExampleAIClient.exe: ExampleAIClient.cpp
	$(CXX) $(CXXFLAGS) $< $(CXXLIBS) -o $@

# $(OBJECTPATH)%.o: $(CLIENTPATH)%.cpp
	# $(CXX) $(CXXFLAGS) -c $< -o $@

# $(OBJECTPATH)%.o: $(LIBPATH)%.cpp
	# $(CXX) $(CXXFLAGS) -c $< -o $@

# $(OBJECTPATH)%.o: $(SHAREDPATH)%.cpp
	# $(CXX) $(CXXFLAGS) -c $< -o $@

# $(OBJECTPATH)%.o: $(UTILPATH)Util/%.cpp
	# $(CXX) $(CXXFLAGS) -c $< -o $@

# $(OBJECTPATH)UnitCommand.o: $(LIBPATH)../UnitCommand.cpp
	# $(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJECTS): | $(OBJECTPATH)

$(OBJECTPATH):
	mkdir $(OBJECTPATH)

clean:
	rm -rf $(OBJECTPATH) *.exe
