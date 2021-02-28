
CC=gcc
CPP=g++
OBJECTS=$(shell ls -1 src/*.cc | sed 's/cc$$/o/g')
WRENOBJECTS=$(shell ls -1 wren/src/vm/*.c wren/src/optional/*.c | sed 's/c$$/o/g')

dev: CFLAGS=-O1 -std=c++17 -g -Wall -Werror
dev: LFLAGS=-lm -lGL -lpthread -ldl -lrt -lX11
dev: imgui/imgui.o src/raylib-ex.o src/raylib-glfw.o $(OBJECTS) $(WRENOBJECTS)
	$(CPP) $(CFLAGS) -o factropy src/*.o imgui/imgui.o $(WRENOBJECTS) $(LFLAGS)

rel: CFLAGS=-O3 -flto -std=c++17 -g -Wall
rel: LFLAGS=-lm -lGL -lpthread -ldl -lrt -lX11
rel: imgui/imgui.o src/raylib-ex.o src/raylib-glfw.o $(OBJECTS) $(WRENOBJECTS)
	$(CPP) $(CFLAGS) -o factropy src/*.o imgui/imgui.o $(WRENOBJECTS) $(LFLAGS)

src/main.o: src/main.cc
	$(CPP) $(CFLAGS) -c $< -o $@

src/%.o: src/%.cc src/%.h
	$(CPP) $(CFLAGS) -c $< -o $@

clean:
	rm -f factropy src/*.o
	rm -f imgui/imgui.o
	rm -f $(WRENOBJECTS)

clean-dev:
	rm -f main src/*.o

imgui/imgui.o: CFLAGS=-O3 -std=c++11 -g -Wall -Iimgui -Iraylib/src/external -Iraylib/src/external/glfw/include
imgui/imgui.o: imgui/build.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

# Pull Raylib in directly instead of building the separate lib so we can get at some internals
src/raylib-ex.o: DENOISE=-Wno-unused-function -Wno-unused-result -Wno-unused-but-set-variable -Wno-unused-variable
src/raylib-ex.o: CFLAGS=-O3 -std=c99 -g -Wall $(DENOISE) -D_POSIX_C_SOURCE=199309L -Iraylib/src -Iraylib/src/external/glfw/include
src/raylib-ex.o: src/raylib-ex.c src/raylib-ex.h
	$(CC) $(CFLAGS) -c $< -o $@

src/raylib-glfw.o: CFLAGS=-O3 -std=c99 -g -Wall -D_POSIX_C_SOURCE=199309L -Iraylib/src/external/glfw/src
src/raylib-glfw.o: raylib/src/rglfw.c
	$(CC) $(CFLAGS) -c $< -o $@

wren/src/vm/%.o: CFLAGS=-O3 -std=c99 -g -Wall -Iwren/src/include -Iwren/src/optional -Iwren/src/vm
wren/src/vm/%.o: wren/src/vm/%.c wren/src/vm/%.h
	$(CC) $(CFLAGS) -c $< -o $@

wren/src/optional/%.o: CFLAGS=-O3 -std=c99 -g -Wall -Iwren/src/include -Iwren/src/optional -Iwren/src/vm
wren/src/optional/%.o: wren/src/optional/%.c wren/src/optional/%.h
	$(CC) $(CFLAGS) -c $< -o $@
