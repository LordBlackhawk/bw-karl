# The user.mk is used for adaption to a computer configuration.
# You can specify STARCRAFTPATH, CPPCHECK, MAKEFLAGS, DEPLOYMODE.
-include user.mk

LIBRARIES           = utils engine plan expert
EXECUTABLES         = karl tests bwmapmodifier

BASEOUTPATH         = ./out/
COVERAGEPATH        = ./coverage/
LIBPATH             = ./lib/
BOOST_PATH          = ./includes/boost/
MYBWAPIDATAPATH     = ./bwapi-data/
MAPSPATH            = $(MYBWAPIDATAPATH)maps/
CPPCHECKDEFINES     = -UBOOST_BORLAND_DEBUG -UBOOST_DEBUG_PYTHON -UBOOST_ALL_DYN_LINK -U__BORLANDC__ -U_RTLDLL -UBOOST_ABI_PREFIX -UBOOST_ABI_SUFFIX -UBOOST_ASSERT_CONFIG
KARLPARAMS          = --hud --speed=0 --secure --webgui --disable GiveUpExpert

LIBFILES            = $(addprefix $(LIBPATH)lib, $(addsuffix .a, $(LIBRARIES)))
COVERAGELIBFILES    = $(addprefix $(LIBPATH)libcoverage_, $(addsuffix .a, $(LIBRARIES)))
EXEFILES            = $(addsuffix .exe, $(EXECUTABLES))
MODULES             = $(LIBRARIES) $(EXECUTABLES)
MODULEFILES         = $(LIBFILES) $(EXEFILES)

ifndef CPPCHECK
    CPPCHECK = cppcheck
endif

ifndef STARCRAFTMAPSPATH
ifdef STARCRAFTPATH
    STARCRAFTMAPSPATH = $(STARCRAFTPATH)Maps/bw-karl/
endif
endif

ifndef BWAPIDATAPATH
ifdef STARCRAFTPATH
    BWAPIDATAPATH = $(STARCRAFTPATH)bwapi-data/
endif
endif

ifndef DEPLOYMODE
    DEPLOYMODE = single
endif

ifndef DONOTBUILD
    KARLEXE   = karl.exe
    TESTSEXE  = tests.exe
else
    KARLEXE   = karl.sh
endif

all: $(MODULEFILES)

run: $(KARLEXE) deploy-$(DEPLOYMODE)
	$< --parallel $(KARLPARAMS)

runseq: $(KARLEXE) deploy-$(DEPLOYMODE)
	$< $(KARLPARAMS)

debug: $(KARLEXE) deploy-$(DEPLOYMODE)
	gdb --args $< $(KARLPARAMS)

test-smart-turn: $(KARLEXE) $(STARCRAFTMAPSPATH)test-smart-turn-around.scx deploy-test-smart-turn
	$< --parallel --hud --speed=100 --only TestSmartTurnAroundExpert
	
ifndef EXPERIMENT_UNITTYPE
    EXPERIMENT_UNITTYPE="Terran Marine"
endif
ifndef EXPERIMENT_REPETITIONS
    EXPERIMENT_REPETITIONS=2
endif
ifdef EXPERIMENT_NODRAW
    EXPERIMENT_NODRAW=--nodraw
endif
learning-fight-winnable: $(KARLEXE) $(STARCRAFTMAPSPATH)learn-fight-winnable-1.scx $(STARCRAFTMAPSPATH)learn-fight-winnable-2.scx $(STARCRAFTMAPSPATH)learn-fight-winnable-3.scx $(STARCRAFTMAPSPATH)learn-fight-winnable-4.scx deploy-learning-fight-winnable
	$< --parallel --hud --speed=0 --only LearningFightWinnableExperimentExpert --experiment sametype --unittype $(EXPERIMENT_UNITTYPE) --mappath $(STARCRAFTMAPSPATH) --repetitions $(EXPERIMENT_REPETITIONS) $(EXPERIMENT_NODRAW)

test: $(TESTSEXE)
	@echo ' ##############################################################################'
	@$< -p

showtest: $(TESTSEXE)
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
	rm -f $(MODULEFILES)
	rm -rf doxygen/html

lines:
	wc -l sources/*/*.*pp

$(BASEOUTPATH):
	mkdir $(BASEOUTPATH)

$(COVERAGEPATH):
	mkdir $(COVERAGEPATH)

$(STARCRAFTMAPSPATH):
	mkdir $(STARCRAFTMAPSPATH)

CPPCHECK-exists: ; @command -v $(CPPCHECK) > /dev/null || ( echo "cppcheck is required." && exit 1 )

cppcheck: CPPCHECK-exists
	$(CPPCHECK) -j4 -I./includes/ -i$(BOOST_PATH) --std=c++11 --enable=all --max-configs=1 --library=std $(CPPCHECKDEFINES) sources/*/*.cpp --xml 2> cppcheck-errors.xml

$(STARCRAFTMAPSPATH)%.scx: $(MAPSPATH)%.scx | $(STARCRAFTMAPSPATH)
	cp $< $@
	
$(STARCRAFTMAPSPATH)learn-fight-winnable-%.scx: $(MAPSPATH)template-fight-winnable.scx | $(STARCRAFTMAPSPATH)
	cp $< $@

ifdef STARCRAFTPATH
deploy-%: $(MYBWAPIDATAPATH)bwapi-%.ini
	cp -f $< $(BWAPIDATAPATH)bwapi.ini
else
deploy-%: $(MYBWAPIDATAPATH)bwapi-%.ini
	@echo "Unable to deploy since STARCRAFTPATH is unknown."
endif

.FORCE:
