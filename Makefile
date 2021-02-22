
CPP=g++
OBJECTS=$(shell ls -1 *.cc | sed 's/cc$$/o/g')

dev: CFLAGS=-O1 -std=c++17 -g -Wall -Werror -I$(HOME)/include
dev: LFLAGS=-lm -lGL -lpthread -ldl -lrt -lX11 -L$(HOME)/lib -Wl,-rpath,$(HOME)/lib -lwren
dev: imgui/imgui.o $(OBJECTS)
	$(CPP) $(CFLAGS) -o main *.o imgui/imgui.o $(HOME)/lib/libraylib.a $(LFLAGS)

rel: CFLAGS=-O3 -flto -std=c++17 -g -Wall -I$(HOME)/include
rel: LFLAGS=-lm -lGL -lpthread -ldl -lrt -lX11 -L$(HOME)/lib -Wl,-rpath,$(HOME)/lib -lwren
rel: imgui/imgui.o $(OBJECTS)
	$(CPP) $(CFLAGS) -o main *.o imgui/imgui.o $(HOME)/lib/libraylib.a $(LFLAGS)

main.o: main.cc
	$(CPP) $(CFLAGS) -c $< -o $@

%.o: %.cc %.h
	$(CPP) $(CFLAGS) -c $< -o $@

clean:
	rm -f main *.o
	rm -f imgui/imgui.o

imgui/imgui.o: CFLAGS=-O1 -std=c++11 -g -Wall -Iimgui -I$(HOME)/src/raylib/src/external -I$(HOME)/src/raylib/src/external/glfw/include
imgui/imgui.o: imgui/build.cpp
	$(CPP) $(CFLAGS) -c $< -o $@