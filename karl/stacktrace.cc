#include "stacktrace.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

std::string debugfilename;

void setDebugInfoFile(const char* filename)
{
    debugfilename = filename;
}

namespace {
    bool readSingle(DebugInfo& di, FILE* f)
    {
        char buf1[1024], buf2[1024];
        int read = fscanf(f, "%1023s\n%1023s\n", &buf1[0], &buf2[0]);
        
        if ((read < 2) || (strlen(buf2) < 3))
            return false;

        buf2[1023] = 0;
        int p = 2;
        while ((buf2[p] != ':') && (buf2[p] != 0))
            ++p;

        if (buf2[p] != ':')
            return false;

        di.line   = atoi(&buf2[p+1]);
        buf2[p]    = 0;

        di.method = buf1;
        di.file   = buf2;
        
        return true;
    }
}

DebugInfo readDebugInfo(void* addr)
{
    DebugInfo res;
  
    if (debugfilename == "")
        return res;

    char filename[1024];
    sprintf(filename, "addr2line -C -e %s -f -s -i %lx", debugfilename.c_str(), (unsigned long) addr);
    FILE* f = popen (filename, "r");

    if (f == NULL)
        return res;
    
    if (readSingle(res, f)) {
        DebugInfo* outer = &res;
        DebugInfo inner;
        while (readSingle(inner, f)) {
            outer->inlined = new DebugInfo(inner);
            outer = outer->inlined;
        }
    }
    
    pclose(f);
    return res;
}

void releaseDebugInfo(DebugInfo& di)
{
    if (di.inlined != NULL) {
        releaseDebugInfo(*di.inlined);
        delete di.inlined;
        di.inlined = NULL;
    }
}

struct frame {
  struct frame *frm;
  void *ret;
};

void getStackTrace(std::vector<DebugInfo>& list)
{
  list.clear();
  struct frame *p = (frame*) __builtin_frame_address(0);
  while (p) {
    list.push_back(readDebugInfo(p->ret));
    DebugInfo* cur = &list.back();
    while ((cur = cur->inlined) != NULL) {
        list.back().inlined = NULL;
        list.push_back(*cur);
    }
    releaseDebugInfo(list.back());
    p = p->frm;
  }
}

void printStarLine()
{
    printf("**********"
           "**********"
           "**********"
           "**********"
           "**********"
           "**********"
           "**********"
           "**********");
}

void plotStackTrace(int skip)
{
  printStarLine();
  printf("Stack Trace:\n");
  printStarLine();

  struct frame *p = (frame*) __builtin_frame_address(0);
  while (skip > 0) {
    p = p->frm;
    --skip;
  }
  
  int i = 0;
  while (p != NULL) {
    DebugInfo info = readDebugInfo(p->ret);
    printf("Level %d: Called from %s (%s:%d) at %p.\n", i++, info.method.c_str(), info.file.c_str(), info.line, p->ret);
    DebugInfo* cur = &info;
    while ((cur = cur->inlined) != NULL) {
        printf("Level %d: Called from %s (%s:%d) at %p [inlined].\n", i++, cur->method.c_str(), cur->file.c_str(), cur->line, p->ret);
    }
    releaseDebugInfo(info);
    p = p->frm;
  }

  printStarLine();
}