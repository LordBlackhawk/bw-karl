-include $(MAKEFILEPATH)variables.mk

LIBNAME      = $(LIBPATH)/lib$(MODULENAME).a

all: $(LIBNAME)

-include $(MAKEFILEPATH)common.mk

$(LIBNAME): $(OBJECTS) | $(LIBPATH)
	ar rvs $@ $(OBJECTS)