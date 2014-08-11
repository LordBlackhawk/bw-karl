-include $(MAKEFILEPATH)variables.mk

LIBNAME      = $(LIBPATH)/lib$(MODULENAME).a
COVLIBNAME   = $(LIBPATH)/libcoverage_$(MODULENAME).a

all: $(LIBNAME)

coverage: $(COVLIBNAME)

-include $(MAKEFILEPATH)common.mk

$(LIBNAME): $(OBJECTS) | $(LIBPATH)
	ar rvs $@ $(OBJECTS)

$(COVLIBNAME): $(COVOBJECTS) | $(LIBPATH)
	ar rvs $@ $(COVOBJECTS)
