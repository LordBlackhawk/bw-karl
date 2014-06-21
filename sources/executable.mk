-include $(MAKEFILEPATH)variables.mk

EXENAME         = $(PROJECTPATH)$(MODULENAME).exe
MODULEDEPSFILES = $(addprefix $(LIBPATH)lib, $(addsuffix .a, $(MODULEDEPS) $(COMPLETEDEPS)))

all: $(EXENAME)

-include $(MAKEFILEPATH)common.mk

$(EXENAME): $(OBJECTS) $(MODULEDEPSFILES)
	$(CXX) $(OBJECTS) $(CXXLIBS) -Wl,-whole-archive $(addprefix -l, $(COMPLETEDEPS)) -Wl,-no-whole-archive $(addprefix -l, $(MODULEDEPS)) -o $@