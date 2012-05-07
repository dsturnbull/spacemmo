%{
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "src/lib/spacemmo.h"
#include "src/lib/cpu/sasm/sasm.h"

extern FILE *yyin;
extern int yylineno;
extern char *yytext;
sasm_t *ysasm;

struct yfile {
	struct yfile *next;
	char *fn;
};

struct yfile *files;
struct yfile *file;

void
yyerror(const char *str)
{
	fprintf(stderr, "%s: \"%s\"\n", str, yytext);
	exit(1);
}

int
yywrap()
{
	fclose(yyin);
	if (files) {
		yyin = fopen(files->fn, "r");
		free(files);
		files = files->next;
		return 0;
	} else {
		return 1;
	}
}

%}

%union
{
	uint64_t number;
	char *string;
	uint8_t width;
}

%token QCOLON QNEWLINE QCOMMENT QEQU QEQUREL QINCLUDE
%token QNOP QHLT QLOAD QSTORE QADD QSUB QMUL QDIV QAND QOR
%token QJMP QJE QJNE QJZ QJNZ QCALL QRET QDUP QPUSH QPOP QSWAP QINT

%token <number>	QNUMBER
%token <string>	QTEXT
%token <string>	QQTEXT
%token <width> QSTYPE
%token <width> QDTYPE
%token <width> QRTYPE

%%

stmts:
	|
	stmts stmt
	;

stmt:
	  comment
	| lbl_def
	| include
	| data | data_str | data_chr
	| equ  | equrel
	| res
	| push | pusht | pushv | pushtv
	| pop  | popt
	| dup  | dupt
	| swap | swapt
	| load | loadt
	| store | storet
	| add  | addt
	| sub  | subt
	| mul  | mult
	| div  | divt
	| and  | andt
	| or   | ort
	| jmp  | je | jet | jne | jnet | jz | jzt | jnz | jnzt
	| call | ret
	| int
	| nop
	| hlt
	| QNEWLINE

comment:
	QCOMMENT QNEWLINE {};

lbl_def:
	QTEXT QCOLON QNEWLINE {
		slog("lbl_def\n");
		define_label(ysasm, $1);
		free($1);
		slog("\n\n");
	};

include:
	QINCLUDE QQTEXT QNEWLINE
	{
		slog("include\n");
		if (files == NULL) {
			files = calloc(1, sizeof(struct yfile));
			file = files;
		} else {
			file->next = calloc(1, sizeof(struct yfile));
			file = file->next;
		}

		// remove quotes
		$2++;
		$2[strlen($2) - 1] = '\0';
		file->fn = strdup($2);
		slog("\n\n");
	};

data:
	QDTYPE QTEXT QNUMBER QNEWLINE {
		slog("data\n");
		define_variable(ysasm, $2, $3, $1);
		slog("\n\n");
	};

data_str:
	QDTYPE QTEXT QQTEXT QNEWLINE {
		slog("data_str\n");
		// remove quotes
		$3++;
		$3[strlen($3) - 1] = '\0';
		define_data(ysasm, $2, (uint8_t *)$3, strlen($3));
		slog("\n\n");
	};

data_chr:
	QDTYPE QTEXT QTEXT QNEWLINE {
		slog("data_chr\n");
		define_variable(ysasm, $2, $3[0], sizeof(uint8_t));
		slog("\n\n");
	};

equ:
	QTEXT QEQU QNUMBER QNEWLINE {
		slog("equ\n");
		variable_t *var = define_constant(ysasm, $1, $3);
		var->value = $3;
		slog("\n\n");
	};

equrel:
	QTEXT QEQU QEQUREL QTEXT QNEWLINE {
		slog("equrel\n");
		variable_t *other = find_variable(ysasm, $4);
		slog("%lu\n", other->len);
		variable_t *var = define_constant(ysasm, $1, 0);
		var->value = other->len;
		slog("\n\n");
	};

res:
	QRTYPE QTEXT QNUMBER QNEWLINE {
		slog("res\n");
		uint8_t *data = calloc($1, $3);
		define_data(ysasm, $2, data, $3 * $1);
		free(data);
		slog("\n\n");
	};

push:
	QPUSH QNUMBER QNEWLINE {
		slog("push\n");
		push1(ysasm, PUSH, &$2, 8);
		slog("\n\n");
	};

pusht:
	QPUSH QSTYPE QNUMBER QNEWLINE {
		slog("pusht\n");
		push1(ysasm, PUSH, &$3, $2);
		slog("\n\n");
	};

pushv:
	QPUSH QTEXT QNEWLINE {
		slog("pushv\n");
		variable_t *var = find_or_create_variable(ysasm, $2);
		add_variable_ref(ysasm, var, ysasm->ip - ysasm->prog + 1, 8);
		push1(ysasm, PUSH, &var->addr, 8);
		slog("\n\n");
	};

pushtv:
	QPUSH QSTYPE QTEXT QNEWLINE {
		slog("pushtv\n");
		variable_t *var = find_or_create_variable(ysasm, $3);
		add_variable_ref(ysasm, var, ysasm->ip - ysasm->prog + 1, $2);
		push1(ysasm, PUSH, &var->addr, $2);
		slog("\n\n");
	};

pop:
	QPOP QNEWLINE {
		push0(ysasm, POP, 8);
	};

popt:
	QPOP QSTYPE QNEWLINE {
		push0(ysasm, POP, $2);
	};

dup:
	QDUP QNEWLINE {
		push0(ysasm, DUP, 8);
	};

dupt:
	QDUP QSTYPE QNEWLINE {
		push0(ysasm, DUP, $2);
	};

swap:
	QSWAP QNEWLINE {
		push0(ysasm, SWAP, 8);
	};

swapt:
	QSWAP QSTYPE QNEWLINE {
		push0(ysasm, SWAP, $2);
	};

load:
	QLOAD QNEWLINE {
		push0(ysasm, LOAD, 8);
	};

loadt:
	QLOAD QSTYPE QNEWLINE {
		push0(ysasm, LOAD, $2);
	};

store:
	QSTORE QNEWLINE {
		push0(ysasm, STORE, 8);
	};

storet:
	QSTORE QSTYPE QNEWLINE {
		push0(ysasm, STORE, $2);
	};

add:
	QADD QNEWLINE {
		push0(ysasm, ADD, 8);
	};

addt:
	QADD QSTYPE QNEWLINE {
		push0(ysasm, ADD, $2);
	};

sub:
	QSUB QNEWLINE {
		push0(ysasm, SUB, 8);
	};

subt:
	QSUB QSTYPE QNEWLINE {
		push0(ysasm, SUB, $2);
	};

mul:
	QMUL QNEWLINE {
		push0(ysasm, MUL, 8);
	};

mult:
	QMUL QSTYPE QNEWLINE {
		push0(ysasm, MUL, $2);
	};

div:
	QDIV QNEWLINE {
		push0(ysasm, DIV, 8);
	};

divt:
	QDIV QSTYPE QNEWLINE {
		push0(ysasm, DIV, $2);
	};

and:
	QAND QNEWLINE {
		push0(ysasm, AND, 8);
	};

andt:
	QAND QSTYPE QNEWLINE {
		push0(ysasm, AND, $2);
	};

or:
	QOR QNEWLINE {
		push0(ysasm, OR, 8);
	};

ort:
	QOR QSTYPE QNEWLINE {
		push0(ysasm, OR, $2);
	};

jmp:	QJMP	QNEWLINE { push0(ysasm, JMP,	1); };

je:
	QJE QNEWLINE {
		push0(ysasm, JE, 8);
	};

jet:
	QJE QSTYPE QNEWLINE {
		push0(ysasm, JE, $2);
	};

jne:
	QJNE QNEWLINE {
		push0(ysasm, JNE, 8);
	};

jnet:
	QJNE QSTYPE QNEWLINE {
		push0(ysasm, JNE, $2);
	};

jz:
	QJZ QNEWLINE {
		push0(ysasm, JZ, 8);
	};

jzt:
	QJZ QSTYPE QNEWLINE {
		push0(ysasm, JZ, $2);
	};

jnz:
	QJNZ QNEWLINE {
		push0(ysasm, JNZ, 8);
	};

jnzt:
	QJNZ QSTYPE QNEWLINE {
		push0(ysasm, JNZ, $2);
	};

ret:	QRET	QNEWLINE { push0(ysasm, RET,	1); };
call:	QCALL	QNEWLINE { push0(ysasm, CALL,	8); };
nop:	QNOP	QNEWLINE { push0(ysasm, NOP,	1); };
hlt:	QHLT	QNEWLINE { push0(ysasm, HLT, 	1); };
int:	QINT	QNEWLINE { push0(ysasm, INT, 	1); };

%%

