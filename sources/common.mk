$(OBJECTPATH)%.o: $(SOURCEPATH)%.cpp $(OBJECTPATH)%.d
	$(CXX) --std=c++0x $(CXXFLAGS) -c $< -o $@

$(OBJECTPATH)%.d: $(SOURCEPATH)%.cpp
	@echo 'Calculating Dependencies of "$<"...'
	@$(CXX) --std=c++0x -I$(INCLUDEPATH) -MM -MG $< | sed 's,\($*\)\.o[ :]*,$(OBJECTPATH)\1.o $@ : ,g' | sed 's,boost/.*\.hpp,,g' > $@;

-include $(DEPS)

$(DEPS): | $(OBJECTPATH)

$(OBJECTPATH): | $(BASEOUTPATH)
	mkdir $(OBJECTPATH)

$(BASEOUTPATH):
	mkdir $(BASEOUTPATH)

cleandep:
	rm -rf $(OBJECTPATH)*.d

clean:
	rm -rf $(OBJECTPATH)
