-include user.mk

LIBRARIES           = utils engine plan expert
EXECUTABLES         = karl tests

BASEOUTPATH         = ./out/
COVERAGEPATH        = ./coverage/
LIBPATH             = ./lib/
LIBFILES            = $(addprefix $(LIBPATH)lib, $(addsuffix .a, $(LIBRARIES)))
COVERAGELIBFILES    = $(addprefix $(LIBPATH)libcoverage_, $(addsuffix .a, $(LIBRARIES)))
EXEFILES            = $(addsuffix .exe, $(EXECUTABLES))
MODULES             = $(LIBRARIES) $(EXECUTABLES)
MODULEFILES         = $(LIBFILES) $(EXEFILES)

all: $(MODULEFILES)

run: karl.exe
	$< --hud --speed=0 --parallel --webgui
# --report=1000

debug: karl.exe
	gdb --args $< --hud --speed=0

test: tests.exe
	@echo ' ##############################################################################'
	@$< -p

showtest: tests.exe
	@echo ' ##############################################################################'
	$< -l test_suite

testcov: coverage_tests.exe
	$< -p
	@for module in $(LIBRARIES) tests; do \
		$(MAKE) $(MAKEFLAGS) -C ./sources/$$module/ gen-coverage ; \
	done

DOXYGEN-exists: ; @command -v doxygen > /dev/null || ( echo "Doxygen is required to build the docs." && exit 1 )
DOT-exists: ; @command -v dot > /dev/null || ( echo "Dot (from graphviz package) is required to build class hierarchy graphs in the docs." && exit 1 )

doc: DOXYGEN-exists DOT-exists
	@echo "Generating documentation in ./doxygen/html..."
	@doxygen ./doxygen/Doxyfile

define LIB_template
lib/lib$(1).a: .FORCE | $(BASEOUTPATH)
	@$$(MAKE) $$(MAKEFLAGS) -C ./sources/$(1)/ all
lib/libcoverage_$(1).a: .FORCE | $(BASEOUTPATH)
	@$$(MAKE) $$(MAKEFLAGS) -C ./sources/$(1)/ coverage
endef

define EXE_template
$(1).exe: $(LIBFILES) .FORCE | $(BASEOUTPATH)
	@$$(MAKE) $$(MAKEFLAGS) -C ./sources/$(1)/ all
coverage_$(1).exe: $(COVERAGELIBFILES) .FORCE | $(BASEOUTPATH) $(COVERAGEPATH)
	@$$(MAKE) $$(MAKEFLAGS) -C ./sources/$(1)/ coverage
endef

$(foreach lib,$(LIBRARIES),$(eval $(call LIB_template,$(lib))))
$(foreach exe,$(EXECUTABLES),$(eval $(call EXE_template,$(exe))))

cleancov:
	rm -rf $(COVERAGEPATH) $(BASEOUTPATH)coverage_*/ $(LIBPATH)libcoverage_*.a

clean: cleancov
	rm -rf $(BASEOUTPATH)
	rm $(MODULEFILES)
	rm -rf doxygen/html

lines:
	wc -l sources/*/*.*pp

$(BASEOUTPATH):
	mkdir $(BASEOUTPATH)

$(COVERAGEPATH):
	mkdir $(COVERAGEPATH)

CPPCHECKDEFINES = -UBOOST_BORLAND_DEBUG -UBOOST_DEBUG_PYTHON -UBOOST_ALL_DYN_LINK -U__BORLANDC__ -U_RTLDLL -UBOOST_ABI_PREFIX -UBOOST_ABI_SUFFIX -UBOOST_ASSERT_CONFIG
BOOST_PATH      = ./includes/boost/
CPPCHECK        = /f/cppcheck/cppcheck.exe

cppcheck:
	$(CPPCHECK) -j4 -I./includes/ -i$(BOOST_PATH) --std=c++11 --enable=all --max-configs=1 --library=std $(CPPCHECKDEFINES) sources/*/*.cpp --xml 2> cppcheck-errors.xml

.FORCE:
