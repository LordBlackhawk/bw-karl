OBJECTPATH   = out/
BWAPIPATH    = ../bwapi/trunk/bwapi/include/
BWTAPATH     = ../bwta/include/
BOOSTPATH    = ../boost_1_46_1/

CXX          = g++
CXXFLAGS     = -Wall -Wextra -O2 --std=c++0x -I$(BWAPIPATH) -I$(BWTAPATH) -I$(BOOSTPATH)
CXXLIBS      = -L. -lBWAPI -lBWTA

SOURCES      = $(wildcard */*.cpp)
EXECUTEABLES = $(notdir $(SOURCES:.cpp=.exe))
DEPS         = $(addprefix $(OBJECTPATH), $(notdir $(SOURCES:.cpp=.d)))

all: $(EXECUTEABLES) $(DEPS)

echo:
	@echo $(SOURCES)

%.exe: $(OBJECTPATH)%.o
	$(CXX) $(CXXFLAGS) $< $(CXXLIBS) -o $@
	
$(OBJECTPATH)%.o: */%.cpp $(OBJECTPATH)%.d
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(OBJECTPATH)%.d: */%.cpp
	@echo 'Calculating Dependencies of "$<"...'
	@$(CXX) -std=c++0x -MM -MG $< | sed 's,\($*\)\.o[ :]*,$(OBJECTPATH)\1.o $@ : ,g' | sed 's,boost/.*\.hpp,,g' > $@;
	
-include $(DEPS)

$(DEPS): | $(OBJECTPATH)

$(OBJECTPATH):
	mkdir $(OBJECTPATH)

clean:
	rm -rf $(OBJECTPATH) $(EXECUTEABLES)
