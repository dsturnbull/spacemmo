CC=clang
CFLAGS+=-O0 -g
CFLAGS+=-mtune=core2 -fcommon -pipe
CFLAGS+=-pedantic-errors -Wall -Werror -Wextra
CFLAGS+=-Wformat=2 -Wswitch-enum -Wswitch
CFLAGS+=-Wno-unused-variable -Wno-unused-parameter
CFLAGS+=-std=c99
CFLAGS+=-I. -I/usr/local/include/agar

LIBTOOL_FLAGS=-macosx_version_min 10.7 -undefined suppress -flat_namespace 
LDFLAGS+=-L. -ledit
CL_LDFLAGS=$(LDFLAGS) -lSDL -framework Cocoa -framework OpenGL -lagar
SV_LDFLAGS=$(LDFLAGS)

CL=client
CL_SRCS=src/client.c $(src/client/*.c)
CL_OBJS=$(CL_SRCS:%.c=%.o)

SV=server
SV_SRCS=src/server.c $(src/server/*.c)
SV_OBJS=$(SV_SRCS:%.c=%.o)

LIB=libspacemmo.dylib
LIB_SRCS=$(wildcard src/lib/*.c)
LIB_OBJS=$(LIB_SRCS:%.c=%.o)

SRCS=$(CL_SRCS) $(SV_SRCS) $(LIB_SRCS)
OBJS=$(SRCS:%.c=%.o)
DEPS=$(SRCS:%.c=%.d)

SDLMAIN=src/SDLMain.o

all: $(SDLMAIN) $(LIB) $(CL) $(SV)

$(SDLMAIN):
	$(CC) $(CFLAGS) -I/usr/local/include/SDL -c $(@:%.o=%.m) -o $@

$(CL): $(CL_OBJS)
	$(CC) $(CL_LDFLAGS) -Lsrc -lspacemmo $^ $(SDLMAIN) -o $@

$(SV): $(SV_OBJS)
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

