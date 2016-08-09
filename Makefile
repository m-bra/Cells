EXECUTABLE=run
SOURCES=src/main.cpp src/physics/physics.cpp src/graphics/graphics.cpp GLL++/Program.cpp
HEADERS=src/util/vector.h src/linmath.h src/physics/physics.hpp src/util/util.h shared/sleep.h GLL++/GLL/GLL.hpp
CC=g++
CFLAGS=-O3
LDFLAGS=`pkg-config --static --libs glfw3` -lglbinding

OBJECTS=$(SOURCES:%=build/%.o)

SEARCH:=%PROJECT%
CFLAGS+=$(subst $(SEARCH),.,$(shell cat .includes))

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o$(EXECUTABLE) $(OBJECTS) $(LDFLAGS)

build/%.o: % $(HEADERS_PATHS)
	$(CC) $(CFLAGS) -o$@ -c $< 

clean:
	mkdir -p backup
	cp -tbackup $(OBJECTS) $(EXECUTABLE) $(SOURCES_PATHS) $(HEADERS_PATHS)
	rm -f $(OBJECTS)
	rm -f $(EXECUTABLE)

backup:
	mkdir -p backup
	cp -tbackup $(OBJECTS) $(EXECUTABLE) $(SOURCES_PATHS) $(HEADERS_PATHS)
