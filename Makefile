
OBJECTS=$(shell ls -1 *.cc | sed 's/cc$$/o/g')

dev: CFLAGS=-O1 -std=c++17 -g -Wall -Werror -I$(HOME)/include
dev: LFLAGS=-lm -flto $(shell pkg-config --libs cairo freetype2) -lGL -lm -lpthread -ldl -lrt -lX11 -L$(HOME)/lib -Wl,-rpath,$(HOME)/lib -lwren 
dev: nuklear/nuklear.o $(OBJECTS)
	g++ $(CFLAGS) -o main *.o nuklear/nuklear.o $(HOME)/lib/libraylib.a $(LFLAGS)

main.o: main.cc
	g++ $(CFLAGS) -c $< -o $@

%.o: %.cc %.h
	g++ $(CFLAGS) -c $< -o $@

clean:
	rm -f main *.o
	#nuklear/nuklear.o

compat:
	$(MAKE) clean
	$(MAKE) dev CC=g++
	$(MAKE) clean
	$(MAKE) dev CC=clang++
	$(MAKE) clean

nuklear/nuklear.o: CFLAGS=-O3 -flto -std=c89 -g -Wall -Wno-unused-variable -Wno-unused-function $(shell pkg-config --cflags cairo freetype2)
nuklear/nuklear.o: nuklear/nuklear.c nuklear/nuklear.h nuklear/nuklear_cairo.h
	gcc $(CFLAGS) -c $< -o $@
