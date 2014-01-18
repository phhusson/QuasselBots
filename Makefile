
CFLAGS=-IQuasselC $(shell pkg-config glib-2.0 --cflags) -std=gnu11
LDLIBS=$(shell pkg-config glib-2.0 --libs)

all: main

QUASSELC_OBJS:=QuasselC/display.o QuasselC/main.o QuasselC/getters.o QuasselC/cmds.o QuasselC/setters.o
OBJS:=main.o notifier.o minbif.o $(QUASSELC_OBJS)

main: $(OBJS)

clean:
	rm -f main $(OBJS)
