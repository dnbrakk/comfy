CC=gcc
CXX=g++
RM=rm -rf

CFLAGS=-w

CPPFLAGS=-fdiagnostics-color=always -std=c++17 -g -w -fpermissive `pkg-config --cflags libcurl`

LDLIBS= -pthread \
	-lstdc++fs \
	`pkg-config --libs libcurl` \
	-lImlib2 \
	-lX11

SRCS=	src/*.cpp \
	src/widgets/*.cpp

OBJS=	build/getopt.o \
	build/gason.o \
	build/termbox.o \
	build/utf8.o

all: directories comfy

directories:
	mkdir -p build

comfy: getopt.o gason.o termbox.o utf8.o
	$(CXX) $(CPPFLAGS) -o comfy $(SRCS) $(OBJS) $(LDLIBS)

getopt.o: getoptpp/getopt_pp.cpp
	$(CXX) -o build/getopt.o -c getoptpp/getopt_pp.cpp

gason.o: gason/src/gason.cpp
	$(CXX) -o build/gason.o -c gason/src/gason.cpp

termbox.o: termbox/src/termbox.c
	$(CC) $(CFLAGS) -o build/termbox.o -c termbox/src/termbox.c

utf8.o: termbox/src/utf8.c
	$(CC) $(CFLAGS) -o build/utf8.o -c termbox/src/utf8.c

clean:
	$(RM) build comfy

