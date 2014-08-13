GRAPHFILES = $(wildcard */*.gv)
PNGFILES   = $(GRAPHFILES:.gv=.png)

all: $(PNGFILES)

%.png: %.gv
	dot -Tpng -o $@ $<
