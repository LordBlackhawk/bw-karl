LIBRARIES   = utils engine plan expert
EXECUTABLES = karl tests
MAKEFLAGS   = -j4

BASEOUTPATH = ./out/
LIBFILES    = $(addprefix ./lib/lib, $(addsuffix .a, $(LIBRARIES)))
EXEFILES    = $(addsuffix .exe, $(EXECUTABLES))
MODULES     = $(LIBRARIES) $(EXECUTABLES)
MODULEFILES = $(LIBFILES) $(EXEFILES)

all: $(MODULEFILES)

run: karl.exe
	$< --hud --speed=0 --parallel

test: tests.exe
	@echo ' ##############################################################################'
	@$< -p

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

lines:
	wc -l sources/*/*.*pp

$(BASEOUTPATH):
	mkdir $(BASEOUTPATH)

.FORCE:
