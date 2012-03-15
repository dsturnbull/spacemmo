CC=clang
CFLAGS+=-O0 -g
CFLAGS+=-mtune=core2 -fcommon -pipe
CFLAGS+=-pedantic-errors -Wall -Werror -Wextra
CFLAGS+=-Wformat=2 -Wswitch-enum -Wswitch
CFLAGS+=-Wno-unused-variable -Wno-unused-parameter
CFLAGS+=-std=c99
CFLAGS+=-I. -I/usr/local/include/agar -I/usr/local/include/SDL
CFLAGS+=$(shell pkg-config libpng --cflags)
CFLAGS+=-Wno-sign-compare

LIBTOOL_FLAGS+=-macosx_version_min 10.7 -undefined suppress -flat_namespace
LIBTOOL_FLAGS+=-lag_core -lag_vg -lag_gui -lag_dev
LIBTOOL_FLAGS+=-L/usr/X11/lib -lfreetype
LDFLAGS+=-L. -ledit
CL_LDFLAGS=$(LDFLAGS)
CL_LDFLAGS+=-lSDL
CL_LDFLAGS+=-framework Cocoa -framework OpenGL
CL_LDFLAGS+=$(shell pkg-config libpng --libs)
SV_LDFLAGS=$(LDFLAGS)

CL=client
CL_SRCS=src/cl.c $(src/client/*.c)
CL_OBJS=$(CL_SRCS:%.c=%.o)

SV=server
SV_SRCS=src/sv.c $(src/server/*.c)
SV_OBJS=$(SV_SRCS:%.c=%.o)

LIB=libspacemmo.dylib
LIB_SRCS=$(wildcard src/lib/*.c src/lib/ui/*.c)
LIB_OBJS=$(LIB_SRCS:%.c=%.o)

SRCS=$(CL_SRCS) $(SV_SRCS) $(LIB_SRCS)
OBJS=$(SRCS:%.c=%.o)
DEPS=$(SRCS:%.c=%.d)

SDLMAIN=src/SDLMain.o

all: $(SDLMAIN) $(LIB) $(CL) $(SV)

$(SDLMAIN):
	$(CC) $(CFLAGS) -I/usr/local/include/SDL -c $(@:%.o=%.m) -o $@

$(CL): $(CL_OBJS) $(LIB)
	$(CC) $(CL_LDFLAGS) -Lsrc -lspacemmo $^ $(SDLMAIN) -o $@

$(SV): $(SV_OBJS) $(LIB)
	$(CC) $(SV_LDFLAGS) -Lsrc -lspacemmo $^ -o $@

$(LIB): $(LIB_OBJS)
	libtool -dynamic -o $@ $(LIBTOOL_FLAGS) $^

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(CL) $(SV) $(LIB)

analyze:
	$(CC) $(CFLAGS) --analyze $(SRCS)

$(DEPS):
	@$(CC) $(CFLAGS) -M -MM -MT $(@:%.d=%.o) $(@:%.d=%.c) > $@

-include $(DEPS)

.PHONY: all clean analyze $(DEPS)

