EXECUTABLE=organisms
SOURCES=src/main.cpp src/physics/physics.cpp src/graphics/graphics.cpp GLL++/Program.cpp src/logic/logic.cpp
SHARED=../shared
HEADERS=src/physics/physics.hpp $(SHARED)/sleep/1/sleep.h GLL++/GLL/GLL.hpp $(SHARED)/Logger/1/Logger.hpp $(SHARED)/algebraic/1/Optional.hpp $(SHARED)/algebraic/1/Iterator.hpp $(SHARED)/slots/1/slots.hpp src/logic/logic.hpp
CC=g++
CFLAGS=-g -Dcimg_display=0
LDFLAGS=`pkg-config --static --libs glfw3` -lglbinding

OBJECTS=$(SOURCES:%=build/%.o) $(SHARED)/Logger/1/Logger.o

SEARCH:=%PROJECT%
CFLAGS+=$(subst $(SEARCH),.,$(shell cat .includes))

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o$(EXECUTABLE) $(OBJECTS) $(LDFLAGS)

build/%.o: % $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o$@ -c $<

.PHONY: run
run: $(EXECUTABLE)
	./$(EXECUTABLE)

.PHONY: rebuild
rebuild:
	$(MAKE) clean
	$(MAKE)

.PHONY: rerun
rerun:
	$(MAKE) clean
	$(MAKE) run

.PHONY: clean
clean:
	mkdir -p backup
	cp -tbackup $(OBJECTS) $(EXECUTABLE) $(SOURCES_PATHS) $(HEADERS_PATHS) | :
	rm -f $(OBJECTS) | :
	rm -f $(EXECUTABLE) | :

.PHONY: backup
backup:
	mkdir -p backup
	cp -tbackup $(OBJECTS) $(EXECUTABLE) $(SOURCES_PATHS) $(HEADERS_PATHS) | :
