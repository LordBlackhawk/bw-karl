-include $(MAKEFILEPATH)variables.mk

EXENAME         = $(PROJECTPATH)$(MODULENAME).exe
MODULEDEPSFILES = $(addprefix $(LIBPATH)lib, $(addsuffix .a, $(MODULEDEPS)))

all: $(EXENAME)

-include $(MAKEFILEPATH)common.mk

$(EXENAME): $(OBJECTS) $(MODULEDEPSFILES)
	$(CXX) $(OBJECTS) $(CXXLIBS) $(addprefix -l, $(MODULEDEPS)) -o $@