
OBJECTS=$(shell ls -1 *.cc | sed 's/cc$$/o/g')

dev: CFLAGS=-O1 -std=c++17 -g -Wall -Werror -I$(HOME)/include
dev: LFLAGS=-lm -flto $(shell pkg-config --libs cairo freetype2) -L$(HOME)/lib -lwren -Wl,-rpath,$(HOME)/lib -lraylib 
dev: nuklear/nuklear.o $(OBJECTS)
	g++ $(CFLAGS) -o main *.o nuklear/nuklear.o $(LFLAGS)

%.o: %.cc
	g++ $(CFLAGS) -c $< -o $@

clean:
	rm -f main *.o nuklear/nuklear.o

compat:
	$(MAKE) clean
	$(MAKE) dev CC=g++
	$(MAKE) clean
	$(MAKE) dev CC=clang++
	$(MAKE) clean

nuklear/nuklear.o: CFLAGS=-O3 -flto -std=c89 -g -Wall -Wno-unused-variable -Wno-unused-function $(shell pkg-config --cflags cairo freetype2)
nuklear/nuklear.o: nuklear/nuklear.c nuklear/nuklear.h nuklear/nuklear_cairo.h
	gcc $(CFLAGS) -c $< -o $@
