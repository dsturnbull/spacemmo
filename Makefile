CC=clang

#OPT=-O3
OPT=-g
CFLAGS+=-pedantic-errors -Wall -Werror -Wextra -Wformat=2 -Wswitch
CFLAGS+=-Wno-unused-variable -Wno-unused-parameter
CFLAGS+=-Wno-objc-protocol-method-implementation
CFLAGS+=-std=c11
CFLAGS+=-I.
CFLAGS+=$(shell pkg-config libpng --cflags)
CFLAGS+=$(shell agar-vg-config --cflags)
CFLAGS+=$(OPT) -mtune=native -fcommon -pipe

LIBTOOL_FLAGS+=-macosx_version_min 10.7 -undefined warning	\
	       -dynamic -flat_namespace
LIBTOOL_FLAGS+=-lag_core -lag_gui -lag_dev

LDFLAGS+=-L. -ledit

CL=client
CL_SRCS=src/cl.c
CL_OBJS=$(CL_SRCS:%.c=%.o)
CL_CFLAGS+=-flto $(OPT)
CL_LDFLAGS+=$(LDFLAGS)

SV=server
SV_SRCS=src/sv.c
SV_OBJS=$(SV_SRCS:%.c=%.o)
SV_CFLAGS+=-flto $(OPT)
SV_LDFLAGS=$(LDFLAGS)

LIB=libspacemmo.dylib
LIB_SRCS=$(wildcard src/lib/*.c src/lib/ui/*.c src/lib/cpu/*.c src/lib/cpu/hardware/*.c)
LIB_OBJS=$(LIB_SRCS:%.c=%.o)

CPU=cpu
CPU_SRCS=src/cpu.c
CPU_OBJS=$(CPU_SRCS:%.c=%.o)
CPU_CFLAGS+=-flto $(OPT)
CPU_LDFLAGS=$(LDFLAGS)

SASM=sasm
SASM_SRCS=src/sasm.c
SASM_OBJS=$(SASM_SRCS:%.c=%.o)
SASM_CFLAGS+=-flto $(OPT)
SASM_LDFLAGS=$(LDFLAGS)

TEST=test_runner
TEST_SRCS=src/tests.c
TEST_OBJS=$(TEST_SRCS:%.c=%.o)
TEST_CFLAGS+=-flto $(OPT)
TEST_LDFLAGS=$(LDFLAGS)

TESTS_SRCS=$(wildcard tests/*.c)
TESTS_OBJS=$(TESTS_SRCS:%.c=%.o)
TESTS_LIBS=$(TESTS_SRCS:%.c=%.dylib)

SRCS=$(CL_SRCS) $(SV_SRCS) $(LIB_SRCS) $(CPU_SRCS) $(SASM_SRCS) $(TEST_SRCS) $(TESTS_SRCS)
OBJS=$(SRCS:%.c=%.o) $(TESTS_LIBS)
DEPS=$(SRCS:%.c=%.d)

all: $(LIB) $(CL) $(SV) $(CPU) $(SASM) $(TEST)

$(CL): $(CL_OBJS) $(LIB)
	$(CC) $(CL_CFLAGS) $(CL_LDFLAGS) -L. -lspacemmo $(CL_OBJS) -o $@

$(SV): $(SV_OBJS) $(LIB)
	$(CC) $(SV_CFLAGS) $(SV_LDFLAGS) -L. -lspacemmo $(SV_OBJS) -o $@

$(CPU): $(CPU_OBJS) $(LIB)
	$(CC) $(CPU_CFLAGS) $(CPU_LDFLAGS) -L. -lspacemmo $(CPU_OBJS) -o $@

$(SASM): $(SASM_OBJS) $(LIB)
	$(CC) $(SASM_CFLAGS) $(SASM_LDFLAGS) -L. -lspacemmo $(SASM_OBJS) -o $@

$(LIB): $(LIB_OBJS)
	libtool -dynamic -o $@ $(LIBTOOL_FLAGS) $(LIB_OBJS)

$(TEST_OBJS): .compiler_flags
$(TEST): $(TEST_OBJS) $(LIB)
	$(CC) $(TEST_CFLAGS) $(TEST_LDFLAGS) -L. -lspacemmo $(TEST_OBJS) -o $@

$(TESTS_OBJS): .compiler_flags
	$(CC) $(CFLAGS) -c $(@:%.o=%.c) -o $@
	libtool -dynamic -o $(@:%.o=%.dylib) $(LIBTOOL_FLAGS) $@

test: $(TESTS_OBJS) $(TEST)
	@./$(TEST)

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(CL) $(SV) $(CPU) $(SASM) $(TEST) $(LIB)

analyze:
	$(CC) $(CFLAGS) --analyze $(SRCS)

$(DEPS):
	@$(CC) $(CFLAGS) -M -MM -MT $(@:%.d=%.o) $(@:%.d=%.c) > $@

-include $(DEPS)

.PHONY: all clean analyze test $(DEPS)

