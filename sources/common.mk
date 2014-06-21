$(OBJECTPATH)%.o: $(SOURCEPATH)%.cpp $(OBJECTPATH)%.d
	$(CXX) --std=c++0x $(CXXFLAGS) -c $< -o $@

$(OBJECTPATH)%.o: $(SOURCEPATH)%.cc $(OBJECTPATH)%.d
	$(CC) --std=c99 $(CCFLAGS) -c $< -o $@

$(OBJECTPATH)%.o: $(SOURCEPATH)%.s
	$(CC) $(ASFLAGS) -o $@ -c $^

$(OBJECTPATH)%.d: $(SOURCEPATH)%.cpp
	@echo 'Calculating Dependencies of "$<"...'
	@$(CXX) --std=c++0x -I$(INCLUDEPATH) -MM -MG $< | sed 's,\($*\)\.o[ :]*,$(OBJECTPATH)\1.o $@ : ,g' | sed 's,boost/.*\.hpp,,g' > $@;

$(OBJECTPATH)%.d: $(SOURCEPATH)%.cc
	@echo 'Calculating Dependencies of "$<"...'
	@$(CC) --std=c99 -MM -MG $< | sed 's,\($*\)\.o[ :]*,$(OBJECTPATH)\1.o $@ : ,g' | sed 's,boost/.*\.hpp,,g' > $@;

-include $(DEPS)

$(DEPS): | $(OBJECTPATH)

$(OBJECTPATH):
	mkdir $(OBJECTPATH)

cleandep:
	rm -rf $(OBJECTPATH)*.d

clean:
	rm -rf $(OBJECTPATH)
