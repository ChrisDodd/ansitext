CC = gcc
CXX = c++
CFLAGS = -std=gnu2x -Wall -Wextra -MMD -MP -g -O0 -D_XOPEN_SOURCE_EXTENDED
CXXFLAGS = -std=gnu++2x -Wall -Wextra -Woverloaded-virtual -g -O0 -D_XOPEN_SOURCE_EXTENDED

all: hexmap mkcolor boxedit hexes test_layout test_linedraw

hexmap: ansi.o vector.o mapedit.o
boxedit: ansi.o boxes.o editline.o layout_string.o linedraw.o links.o vector.o utf8.o
test_layout: layout_string.o vector.o
test_linedraw: ansi.o linedraw.o
tansi: ansi.o

clean:
	rm -f *.o hexmap

%: %.c

ifneq ($(MAKECMDGOALS),clean)
-include *.d
endif
