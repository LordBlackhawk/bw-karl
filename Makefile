-include user.mk

LIBRARIES   = utils engine plan expert
EXECUTABLES = karl tests

BASEOUTPATH = ./out/
LIBFILES    = $(addprefix ./lib/lib, $(addsuffix .a, $(LIBRARIES)))
EXEFILES    = $(addsuffix .exe, $(EXECUTABLES))
MODULES     = $(LIBRARIES) $(EXECUTABLES)
MODULEFILES = $(LIBFILES) $(EXEFILES)

all: $(MODULEFILES)

run: karl.exe
	$< --hud --speed=0
#--parallel

debug: karl.exe
	gdb --args $< --hud --speed=0

test: tests.exe
	@echo ' ##############################################################################'
	@$< -p

showtest: tests.exe
	@echo ' ##############################################################################'
	$< -l test_suite

DOXYGEN-exists: ; @command -v doxygen > /dev/null || ( echo "Doxygen is required to build the docs." && exit 1 )
DOT-exists: ; @command -v dot > /dev/null || ( echo "Dot (from graphviz package) is required to build class hierarchy graphs in the docs." && exit 1 )

doc: DOXYGEN-exists DOT-exists
	@echo "Generating documentation in ./doxygen/html..."
	@doxygen ./doxygen/Doxyfile

define LIB_template
lib/lib$(1).a: .FORCE | $(BASEOUTPATH)
	@$$(MAKE) $$(MAKEFLAGS) -C ./sources/$(1)/ all
endef

define EXE_template
$(1).exe: $(LIBFILES) .FORCE | $(BASEOUTPATH)
	@$$(MAKE) $$(MAKEFLAGS) -C ./sources/$(1)/ all
endef

$(foreach lib,$(LIBRARIES),$(eval $(call LIB_template,$(lib))))
$(foreach exe,$(EXECUTABLES),$(eval $(call EXE_template,$(exe))))

clean:
	rm -rf $(BASEOUTPATH)
	rm $(MODULEFILES)
	rm -rf doxygen/html

lines:
	wc -l sources/*/*.*pp

$(BASEOUTPATH):
	mkdir $(BASEOUTPATH)

.FORCE:
