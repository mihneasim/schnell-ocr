CFLAGS= -Wall -O2 -march=nocona -pipe -fomit-frame-pointer -mmmx -msse -msse2 -msse3

.PHONY: all tags clean

MAKEFILE=Makefile

CFLAGS:=$(CFLAGS) -g

all: hauptteil tools

tags:
	ctags -R `pkg-config --cflags opencv | sed -e 's/^.*-I\([^ \t]*\).*/\1/'` .

distclean:clean
	-rm tags

clean:
	-$(MAKE) -C ocr clean
	-$(MAKE) -C tools clean
	#-rm tags

###############################################################
hauptteil:
	CFLAGS="$(CFLAGS)" $(MAKE) -C ocr
	
###############################################################

tools:
	CFLAGS="$(CFLAGS)" $(MAKE) -C tools
