%{
#include <stdio.h>
#include <string.h>
#include <assert.h>

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
	if (files) {
		yyin = fopen(files->fn, "r");
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

%token QCOLON QNEWLINE QCOMMENT QQUOTE QDQUOTE QEQU QINCLUDE
%token QNOP QHLT QLOAD QSTORE QADD QSUB QMUL QDIV QAND QOR
%token QJMP QJE QJNE QJZ QJNZ QCALL QRET QDUP QPUSH QPOP QSWAP QINT

%token <number>	QNUMBER
%token <string>	QTEXT
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
	| equ
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
		define_label(ysasm, $1);
		free($1);
	};

include:
	QINCLUDE QDQUOTE QTEXT QDQUOTE QNEWLINE
	{
		if (files == NULL) {
			files = calloc(1, sizeof(struct yfile));
			file = files;
		} else {
			file->next = calloc(1, sizeof(struct yfile));
			file = file->next;
		}

		file->fn = strdup($3);
	};

data:
	QDTYPE QTEXT QNUMBER QNEWLINE {
		define_variable(ysasm, $2, $3, $1);
	};

data_str:
	QDTYPE QTEXT QDQUOTE QTEXT QDQUOTE QNEWLINE {
		define_data(ysasm, $2, (uint8_t *)$4, strlen($4));
	};

data_chr:
	QDTYPE QTEXT QQUOTE QTEXT QQUOTE QNEWLINE {
		define_variable(ysasm, $2, $4[0], sizeof(uint8_t));
	};

equ:
	QTEXT QEQU QNUMBER QNEWLINE {
		define_constant(ysasm, $1, $3);
	};

res:
	QRTYPE QTEXT QNUMBER QNEWLINE {
		uint8_t *data = calloc($1, $3);
		define_data(ysasm, $2, data, $3 * $1);
		free(data);
	};

push:
	QPUSH QNUMBER QNEWLINE {
		push1(ysasm, PUSH, &$2, 8);
	};

pusht:
	QPUSH QSTYPE QNUMBER QNEWLINE {
		push1(ysasm, PUSH, &$3, $2);
	};

pushv:
	QPUSH QTEXT QNEWLINE {
		variable_t *var = find_or_create_variable(ysasm, $2);
		add_variable_ref(ysasm, var, ysasm->ip - ysasm->prog + 1);
		push1(ysasm, PUSH, &var->addr, 8);
	};

pushtv:
	QPUSH QSTYPE QTEXT QNEWLINE {
		variable_t *var = find_or_create_variable(ysasm, $3);
		push1(ysasm, PUSH, &var->addr, $2);
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
call:	QCALL	QNEWLINE { push0(ysasm, CALL,	1); };
nop:	QNOP	QNEWLINE { push0(ysasm, NOP,	1); };
hlt:	QHLT	QNEWLINE { push0(ysasm, HLT, 	1); };
int:	QINT	QNEWLINE { push0(ysasm, INT, 	1); };

%%

/*
equrel:
	QTEXT QEQU '$' '-' QNUMBER QNEWLINE {
	};
*/
