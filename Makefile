
CC=gcc
CPP=g++
OBJECTS=$(shell ls -1 *.cc | sed 's/cc$$/o/g')

dev: CFLAGS=-O1 -std=c++17 -g -Wall -Werror -L$(HOME)/lib -I$(HOME)/include
dev: LFLAGS=-lm -lGL -lpthread -ldl -lrt -lX11 -Wl,-rpath,$(HOME)/lib -lwren
dev: imgui/imgui.o raylib-ex.o raylib-glfw.o $(OBJECTS)
	$(CPP) $(CFLAGS) -o main *.o imgui/imgui.o $(LFLAGS)

rel: CFLAGS=-O3 -flto -std=c++17 -g -Wall -L$(HOME)/lib -I$(HOME)/include
rel: LFLAGS=-lm -lGL -lpthread -ldl -lrt -lX11 -Wl,-rpath,$(HOME)/lib -lwren
rel: imgui/imgui.o raylib-ex.o raylib-glfw.o $(OBJECTS)
	$(CPP) $(CFLAGS) -o main *.o imgui/imgui.o $(LFLAGS)

main.o: main.cc
	$(CPP) $(CFLAGS) -c $< -o $@

%.o: %.cc %.h
	$(CPP) $(CFLAGS) -c $< -o $@

clean:
	rm -f main *.o
	rm -f imgui/imgui.o

imgui/imgui.o: CFLAGS=-O3 -std=c++11 -g -Wall -Iimgui -Iraylib/src/external -Iraylib/src/external/glfw/include
imgui/imgui.o: imgui/build.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

# Pull Raylib in directly instead of building the separate lib so we can get at some internals
raylib-ex.o: DENOISE=-Wno-unused-function -Wno-unused-result -Wno-unused-but-set-variable
raylib-ex.o: CFLAGS=-O3 -std=c99 -g -Wall $(DENOISE) -D_POSIX_C_SOURCE=199309L -Iraylib/src -Iraylib/src/external/glfw/include
raylib-ex.o: raylib-ex.c raylib-ex.h
	$(CC) $(CFLAGS) -c $< -o $@

raylib-glfw.o: CFLAGS=-O3 -std=c99 -g -Wall -D_POSIX_C_SOURCE=199309L -Iraylib/src/external/glfw/src
raylib-glfw.o: raylib/src/rglfw.c
	$(CC) $(CFLAGS) -c $< -o $@
