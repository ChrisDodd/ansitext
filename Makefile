CC = gcc
CXX = c++
CFLAGS = -std=gnu11 -Wall -Wextra -MMD -MP -g -O0 -D_XOPEN_SOURCE_EXTENDED
CXXFLAGS = -std=gnu++11 -Wall -Wextra -Woverloaded-virtual -g -O0 -D_XOPEN_SOURCE_EXTENDED

all: hexmap mkcolor

hexmap: ansi.o vector.o mapedit.o

clean:
	rm -f *.o hexmap

%: %.c

ifneq ($(MAKECMDGOALS),clean)
-include *.d
endif
