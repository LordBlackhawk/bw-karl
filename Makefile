SOURCEPATH	 = karl/
OBJECTPATH   = out/
BWAPIPATH    = includes/
BOOSTPATH    = ../boost_1_46_1
LIBPATH		 = .

CXX          = g++ -ggdb
CXXINCLUDES  = -I$(BWAPIPATH) -I$(BOOSTPATH) -I.
CXXFLAGS     = -Wall -Wextra -O3 --std=c++0x $(CXXINCLUDES)
CXXLIBS      = -L$(LIBPATH) -L$(BOOSTPATH)/stage/lib -lBWAPI -lBWTA

SOURCES      = $(wildcard $(SOURCEPATH)*.cpp)
OBJECTS		 = $(addprefix $(OBJECTPATH), $(notdir $(SOURCES:.cpp=.o)))
EXECUTEABLES = karl.exe
DEPS         = $(addprefix $(OBJECTPATH), $(notdir $(SOURCES:.cpp=.d)))

all: $(EXECUTEABLES) $(DEPS)
	
run: karl.exe
	$<
	
debugrun: karl.exe
	gdb $<

echo:
	@echo "Sources:"
	@echo $(SOURCES)
	@echo "Objects:"
	@echo $(OBJECTS)

karl.exe: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(CXXLIBS) -o $@
	
$(OBJECTPATH)%.o: $(SOURCEPATH)%.cpp $(OBJECTPATH)%.d
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(OBJECTPATH)%.d: $(SOURCEPATH)%.cpp
	@echo 'Calculating Dependencies of "$<"...'
	@$(CXX) --std=c++0x -I. -MM -MG $< | sed 's,\($*\)\.o[ :]*,$(OBJECTPATH)\1.o $@ : ,g' | sed 's,boost/.*\.hpp,,g' > $@;
	
-include $(DEPS)

$(DEPS): | $(OBJECTPATH)

$(OBJECTPATH):
	mkdir $(OBJECTPATH)

cleandep:
	rm -rf $(OBJECTPATH)*.d

clean:
	rm -rf $(OBJECTPATH) $(EXECUTEABLES) BWTA.log
