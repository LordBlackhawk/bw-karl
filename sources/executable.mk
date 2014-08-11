-include $(MAKEFILEPATH)variables.mk

EXENAME         = $(PROJECTPATH)$(MODULENAME).exe
COVEXENAME      = $(PROJECTPATH)coverage_$(MODULENAME).exe
MODULEDEPSFILES = $(addprefix $(LIBPATH)lib, $(addsuffix .a, $(MODULEDEPS) $(COMPLETEDEPS)))

all: $(EXENAME)

coverage: $(COVEXENAME)

-include $(MAKEFILEPATH)common.mk

$(EXENAME): $(OBJECTS) $(MODULEDEPSFILES)
	$(CXX) $(OBJECTS) -Wl,-whole-archive $(addprefix -l, $(COMPLETEDEPS)) -Wl,-no-whole-archive $(addprefix -l, $(MODULEDEPS)) $(CXXLIBS) -o $@

$(COVEXENAME): $(COVOBJECTS) $(MODULEDEPSFILES)
	$(CXX) $(COVOBJECTS) -Wl,-whole-archive $(addprefix -lcoverage_, $(COMPLETEDEPS)) -Wl,-no-whole-archive $(addprefix -lcoverage_, $(MODULEDEPS)) $(CXXLIBS) -lgcov -o $@
