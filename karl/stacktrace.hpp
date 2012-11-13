#ifndef STACKTRACE_h
#define STACKTRACE_h

//! Use "g++ -g ..." to add debugging information to executable.

#include <string>
#include <vector>

struct DebugInfo
{
  std::string method;
  std::string file;
  int         line;
  
  DebugInfo*  inlined;

  DebugInfo() : method("??"), file("??"), line(0), inlined(NULL) { }
};

void printStarLine();
void setDebugInfoFile(const char* filename);
DebugInfo readDebugInfo(void* addr);
void releaseDebugInfo(DebugInfo& di);
void getStackTrace(std::vector<DebugInfo>& list);
void plotStackTrace(int skip = 0);

#endif