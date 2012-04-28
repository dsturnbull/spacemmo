CC=clang

#OPT=-O3
OPT=-g
CFLAGS+=-pedantic -Wall  -Wextra -Wformat=2 -Wswitch
CFLAGS+=-Wno-unused-variable -Wno-unused-parameter -Wno-unused-function
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
LIB_SRCS=$(wildcard src/lib/*.c src/lib/ui/*.c src/lib/cpu/*.c src/lib/cpu/hardware/*.c src/lib/cpu/hardware/peripheral/*.c) src/lib/cpu/sasm/sasm.c
LIB_OBJS=$(LIB_SRCS:%.c=%.o)

CPU=cpu
CPU_SRCS=src/cpu.c
CPU_OBJS=$(CPU_SRCS:%.c=%.o)
CPU_CFLAGS+=-flto $(OPT)
CPU_LDFLAGS=$(LDFLAGS)

SASM=sasm
SASM_LEX_SRC=src/lib/cpu/sasm/sasm.l
SASM_LEXER_SRC=src/lib/cpu/sasm/sasm.yy.c
SASM_YACC_SRC=src/lib/cpu/sasm/sasm.y
SASM_YACC_PARSER_SRC=src/lib/cpu/sasm/sasm.y.tab.c
SASM_YACC_PARSER_HDR=$(SASM_YACC_PARSER_SRC:%.c=%.h)
SASM_YACC_PARSER_OBJ=$(SASM_YACC_PARSER_SRC:%.c=%.o)
SASM_SRCS=$(SASM_YACC_PARSER_SRC) $(SASM_LEXER_SRC) src/sasm.c
SASM_OBJS=$(SASM_SRCS:%.c=%.o)
SASM_CFLAGS+=-flto $(OPT)
SASM_LDFLAGS=$(LDFLAGS)

SRCS=$(CL_SRCS) $(SV_SRCS) $(LIB_SRCS) $(CPU_SRCS) $(SASM_SRCS)
OBJS=$(SRCS:%.c=%.o)
DEPS=$(SRCS:%.c=%.d)

all: $(LIB) $(CL) $(SV) $(CPU) $(SASM)

$(CL): $(CL_OBJS) $(LIB)
	$(CC) $(CL_CFLAGS) $(CL_LDFLAGS) -L. -lspacemmo $(CL_OBJS) -o $@

$(SV): $(SV_OBJS) $(LIB)
	$(CC) $(SV_CFLAGS) $(SV_LDFLAGS) -L. -lspacemmo $(SV_OBJS) -o $@

$(CPU): $(CPU_OBJS) $(LIB)
	$(CC) $(CPU_CFLAGS) $(CPU_LDFLAGS) -L. -lspacemmo $(CPU_OBJS) -o $@

%.c: %.y
%.c: %.l

$(SASM_LEXER_SRC): $(SASM_LEX_SRC)
	lex -o $@ $^

$(SASM_YACC_PARSER_SRC): $(SASM_YACC_SRC)
	bison -d -b $^ $^

$(SASM): $(SASM_OBJS) $(LIB)
	$(CC) $(SASM_CFLAGS) $(SASM_LDFLAGS) -L. -lspacemmo $(SASM_OBJS) -o $@

$(LIB): $(LIB_OBJS)
	libtool -dynamic -o $@ $(LIBTOOL_FLAGS) $(LIB_OBJS)

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(CL) $(SV) $(CPU) $(SASM) $(LIB)
	rm -f $(SASM_LEXER_SRC)
	rm -f $(SASM_YACC_PARSER_HDR) $(SASM_YACC_PARSER_SRC)

analyze:
	$(CC) $(CFLAGS) --analyze $(SRCS)

$(DEPS):
	@$(CC) $(CFLAGS) -M -MM -MT $(@:%.d=%.o) $(@:%.d=%.c) > $@

-include $(DEPS)

.PHONY: all clean analyze $(DEPS)

