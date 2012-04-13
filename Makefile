CC=clang

#OPT=-O3
OPT=-g
CFLAGS+=-pedantic-errors -Wall -Werror -Wextra -Wformat=2 -Wswitch
CFLAGS+=-Wno-unused-variable -Wno-unused-parameter -Wno-sign-compare
CFLAGS+=-std=c11
CFLAGS+=-I. -I/usr/local/include/SDL
CFLAGS+=$(shell pkg-config libpng --cflags)
CFLAGS+=$(shell agar-vg-config --cflags)
CFLAGS+=$(OPT) -mtune=native -fcommon -pipe

LIBTOOL_FLAGS+=-macosx_version_min 10.7 -undefined warning	\
	       -dynamic -flat_namespace
LIBTOOL_FLAGS+=-lag_core -lag_gui -lag_dev

LDFLAGS+=-L. -ledit -L/usr/X11/lib -lfreetype

CL=client
CL_SRCS=src/cl.c
CL_OBJS=$(CL_SRCS:%.c=%.o)
CL_CFLAGS+=-flto $(OPT)
CL_LDFLAGS+=$(LDFLAGS)
CL_LDFLAGS+=-lSDL -lSDL_image
CL_LDFLAGS+=-framework Cocoa -framework OpenGL
CL_LDFLAGS+=$(shell pkg-config libpng --libs)

CLI=cli
CLI_SRCS=src/cli.c
CLI_OBJS=$(CLI_SRCS:%.c=%.o)
CLI_CFLAGS+=-flto $(OPT)
CLI_LDFLAGS+=$(LDFLAGS)

SV=server
SV_SRCS=src/sv.c
SV_OBJS=$(SV_SRCS:%.c=%.o)
SV_CFLAGS+=-flto $(OPT)
SV_LDFLAGS=$(LDFLAGS)

MON=mon
MON_SRCS=src/mon.c
MON_OBJS=$(MON_SRCS:%.c=%.o)
MON_CFLAGS+=-flto $(OPT)
MON_LDFLAGS=$(LDFLAGS)

LIB=libspacemmo.dylib
LIB_SRCS=$(wildcard src/lib/*.c src/lib/ui/*.c src/lib/cpu/*.c src/lib/cpu/hardware/*.c)
LIB_OBJS=$(LIB_SRCS:%.c=%.o)

SRCS=$(CL_SRCS) $(SV_SRCS) $(LIB_SRCS)
OBJS=$(SRCS:%.c=%.o)
DEPS=$(SRCS:%.c=%.d)

SDLMAIN=src/SDLMain.o

all: $(SDLMAIN) $(LIB) $(CL) $(SV) $(CLI) $(MON)

$(SDLMAIN):
	$(CC) $(CFLAGS) -I/usr/local/include/SDL -c $(@:%.o=%.m) -o $@

$(CL): $(CL_OBJS) $(LIB)
	$(CC) $(CL_CFLAGS) $(CL_LDFLAGS) -L. -lspacemmo $(CL_OBJS) $(SDLMAIN) -o $@

$(SV): $(SV_OBJS) $(LIB)
	$(CC) $(SV_CFLAGS) $(SV_LDFLAGS) -L. -lspacemmo $(SV_OBJS) -o $@

$(CLI): $(CLI_OBJS) $(LIB)
	$(CC) $(CLI_CFLAGS) $(CLI_LDFLAGS) -L. -lspacemmo $(CLI_OBJS) -o $@

$(MON): $(MON_OBJS) $(LIB)
	$(CC) $(MON_CFLAGS) $(MON_LDFLAGS) -L. -lspacemmo $(MON_OBJS) -o $@

$(LIB): $(LIB_OBJS)
	libtool -dynamic -o $@ $(LIBTOOL_FLAGS) $(LIB_OBJS)

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(CL) $(SV) $(CLI) $(MON) $(LIB)

analyze:
	$(CC) $(CFLAGS) --analyze $(SRCS)

$(DEPS):
	@$(CC) $(CFLAGS) -M -MM -MT $(@:%.d=%.o) $(@:%.d=%.c) > $@

-include $(DEPS)

.PHONY: all clean analyze $(DEPS)

