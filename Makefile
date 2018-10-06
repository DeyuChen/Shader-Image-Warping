CC = g++
CFLAGS = -c -std=c++11
LDFLAGS = -lSDL2 -lglut -lGL -lGLU -lglfw -lGLEW -lavformat -lavcodec -lavutil -lswscale -lSDL2_image -lIL -lILU
SOURCES = main.cpp SDLWindow.cpp
OBJECTS = $(SOURCES:.cpp=.o)

EXECUTABLE = a.out

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *o $(EXECUTABLE)
