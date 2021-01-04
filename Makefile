CC  = gcc
CXX = g++

CFLAGS   = -g -Wall $(INCLUDES)
CXXFLAGS = -g -Wall $(INCLUDES)

.PHONY:default
default: gbnnode dvnode cnnode

cnnode: cnnode.o func.o

gbnnode: gbnnode.o func.o

dvnode: dvnode.o func.o

cnnode.o: func.h

dvnode.o: func.h

gbnnode.o: func.h

func.o: func.h

.PHONY: clean
clean:
	rm -f *.o *~ a.out gbnnode dvnode cnnode

.PHONY: all
all:clean default

