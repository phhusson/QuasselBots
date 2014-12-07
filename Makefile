CFLAGS:=-IQuasselC $(shell pkg-config glib-2.0 --cflags) -std=gnu11 -Wall -Wextra -g
CFLAGS+=$(shell pkg-config lua5.2 --cflags)

LDLIBS:=$(shell pkg-config glib-2.0 --libs) 
LDLIBS+=$(shell pkg-config lua5.2 --libs) -lz

all: main

QUASSELC_OBJS:=QuasselC/display.o QuasselC/main.o QuasselC/getters.o QuasselC/cmds.o QuasselC/setters.o QuasselC/negotiation.o QuasselC/io.o
OBJS:=main.o notifier.o minbif.o lua.o $(QUASSELC_OBJS)

main: $(OBJS)

clean:
	rm -f main $(OBJS)
