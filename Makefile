
CC=g++
OBJECTS=$(shell ls -1 *.cc | sed 's/cc$$/o/g')

dev: CFLAGS=-O1 -std=c++17 -g -Wall -Werror
dev: LFLAGS=-lm -L$(HOME)/lib -Wl,-rpath,$(HOME)/lib -lraylib
dev: $(OBJECTS)
	$(CC) $(CFLAGS) -o main *.o $(LFLAGS)

%.o: %.cc
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f main *.o

compat:
	$(MAKE) clean
	$(MAKE) dev CC=g++
	$(MAKE) clean
	$(MAKE) dev CC=clang++
	$(MAKE) clean
