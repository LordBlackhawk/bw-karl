OBJECTPATH   = out/
BWAPIPATH    = ../bwapi/trunk/bwapi/include/
BWTAPATH     = ../bwta/include/
BOOSTPATH    = ../boost_1_46_1/

CXX          = g++
CXXINCLUDES  = -I$(BWAPIPATH) -I$(BWTAPATH) -I$(BOOSTPATH) -I.
CXXFLAGS     = -Wall -Wextra -O3 --std=c++0x $(CXXINCLUDES)
CXXLIBS      = -L. -lBWAPI -L$(BOOSTPATH)stage/lib -lboost_program_options-mgw46-mt-1_46_1 -lboost_regex-mgw46-mt-1_46_1

ifdef DEBUG
CXX         += -ggdb
CXXLIBS     += -lBWTA-debug
else
CXXLIBS     += -lBWTA
endif

REALSOURCES  = newplan/bwplan.cpp micro/micro.cpp informations/informations.cpp
OBJECTS		 = $(addprefix $(OBJECTPATH), $(notdir $(REALSOURCES:.cpp=.o)))
SOURCES      = $(filter-out $(REALSOURCES),$(wildcard */*.cpp))
EXECUTEABLES = $(notdir $(SOURCES:.cpp=.exe))
DEPS         = $(addprefix $(OBJECTPATH), $(notdir $(SOURCES:.cpp=.d) $(REALSOURCES:.cpp=.d)))

all: $(EXECUTEABLES) $(DEPS)

run: karl.exe
	karl -l builds/zerg/overpool.bwb

echo:
	@echo "Sources:"
	@echo $(SOURCES)
	@echo "Objects:"
	@echo $(OBJECTS)

%.exe: $(OBJECTPATH)%.o
	$(CXX) $(CXXFLAGS) $< $(CXXLIBS) -o $@
	
PlanDesigner.exe: $(OBJECTPATH)PlanDesigner.o out/bwplan.o
	$(CXX) $(CXXFLAGS) out/bwplan.o $< $(CXXLIBS) -o $@
	
karl.exe: $(OBJECTPATH)karl.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $< $(CXXLIBS) -o $@
	
newplan/resourceenum.h: BWPlanWriter.exe
	$< resourceenum.h > $@

newplan/operationenum.h: BWPlanWriter.exe
	$< operationenum.h > $@

newplan/bwplan.cpp: BWPlanWriter.exe
	$< bwplan.cpp > $@
	
$(OBJECTPATH)%.o: */%.cpp $(OBJECTPATH)%.d
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(OBJECTPATH)%.d: */%.cpp
	@echo 'Calculating Dependencies of "$<"...'
	@$(CXX) --std=c++0x -I. -MM -MG $< | sed 's,\($*\)\.o[ :]*,$(OBJECTPATH)\1.o $@ : ,g' | sed 's,boost/.*\.hpp,,g' > $@;
	
-include $(DEPS)

$(DEPS): | $(OBJECTPATH)

$(OBJECTPATH):
	mkdir $(OBJECTPATH)

cleandep:
	rm -rf $(OBJECTPATH)*.d

clean:
	rm -rf $(OBJECTPATH) $(EXECUTEABLES)
