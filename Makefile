LIBRARIES   = utils
EXECUTABLES = karl tests
MAKEFLAGS   = -j4

LIBFILES    = $(addprefix ./lib/lib, $(addsuffix .a, $(LIBRARIES)))
EXEFILES    = $(addsuffix .exe, $(EXECUTABLES))
MODULES     = $(LIBRARIES) $(EXECUTABLES)
MODULEFILES = $(LIBFILES) $(EXEFILES)

all: $(MODULEFILES)

test: tests.exe
	@echo ' ##############################################################################'
	@$< -p

define LIB_template
lib/lib$(1).a: .FORCE
	@$$(MAKE) $$(MAKEFLAGS) -C ./sources/$(1)/ all
endef

define EXE_template
$(1).exe: $(LIBFILES) .FORCE
	@$$(MAKE) $$(MAKEFLAGS) -C ./sources/$(1)/ all
endef

$(foreach lib,$(LIBRARIES),$(eval $(call LIB_template,$(lib))))
$(foreach exe,$(EXECUTABLES),$(eval $(call EXE_template,$(exe))))

clean:
	@for module in $(MODULES); do \
		$(MAKE) -C ./sources/$$module/ clean ; \
	done
	rm $(MODULEFILES)

.FORCE:
