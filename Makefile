CC = gcc
CXX = c++
CFLAGS = -std=gnu2x -Wall -Wextra -MMD -MP -g -O0 -D_XOPEN_SOURCE_EXTENDED -Wno-gnu-alignof-expression
CXXFLAGS = -std=gnu++2x -Wall -Wextra -Woverloaded-virtual -g -O0 -D_XOPEN_SOURCE_EXTENDED

all: hexmap mkcolor boxedit hexes test_layout test_linedraw test_menu

hexmap: ansi.o vector.o mapedit.o
boxedit: ansi.o boxes.o editline.o errors.o layout_string.o linedraw.o links.o menu.o vector.o utf8.o
test_layout: layout_string.o vector.o
test_linedraw: ansi.o linedraw.o
test_menu: ansi.o menu.o layout_string.o vector.o
tansi: ansi.o

clean:
	rm -f *.o hexmap

%: %.c

ifneq ($(MAKECMDGOALS),clean)
-include *.d
endif
