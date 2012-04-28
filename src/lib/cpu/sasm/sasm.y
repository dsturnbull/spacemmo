%{
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "src/lib/cpu/sasm/sasm.h"

extern int yylineno;
extern char *yytext;
sasm_t *ysasm;

void
yyerror(const char *str)
{
	fprintf(stderr, "%s at %d: %s\n", str, yylineno, yytext);
	exit(1);
}

int
yywrap()
{
	return 1;
}

%}

%union
{
	uint64_t number;
	char *string;
}

%token QCOLON QNEWLINE QCOMMENT QQUOTE QDQUOTE
%token QBYTE QWORD QDWORD QQWORD
%token QDB QDW QDD QDQ QEQU
%token QNOP QHLT QLOAD QSTORE QADD QSUB QMUL QDIV QAND QOR
%token QJMP QJE QJZ QJNZ QCALL QRET QDUP QPUSH QPOP QSWAP QINT

%token <number>	QNUMBER
%token <string>	QTEXT

%%

stmts:
	|
	stmts stmt
	;

stmt:
    	  comment
	| lbl_def

	| ret

	| push
	| push_byte	| push_word	| push_dword	| push_qword

	| push_var

	| pop
	| pop_byte	| pop_word	| pop_dword	| pop_qword

	| db	| dw	| dd	| dq	| equ	| equrel
	| db_str| db_chr

	| load
	| load_b	| load_w	| load_d	| load_q

	| nop	| hlt	| store	| add	| sub	| mul	| div
	| and	| or	| jmp	| je	| jz	| jnz	| call
	| dup	| swap	| int

	| QNEWLINE
	;

comment:
	QCOMMENT QNEWLINE {};

lbl_def:
	QTEXT QCOLON QNEWLINE {
		define_label(ysasm, $1);
		free($1);
	};

ret:
	QRET QNEWLINE {
		push0(ysasm, RET, 1);
	};

push:
	QPUSH QNUMBER QNEWLINE {
		push1(ysasm, PUSH, (uint64_t *)&$2, sizeof(uint64_t));
	};

push_byte:
	QPUSH QBYTE QNUMBER QNEWLINE {
		push1(ysasm, PUSH, (uint8_t *)&$3, sizeof(uint8_t));
	};

push_word:
	QPUSH QWORD QNUMBER QNEWLINE {
		push1(ysasm, PUSH, (uint16_t *)&$3, sizeof(uint16_t));
	};

push_dword:
	QPUSH QDWORD QNUMBER QNEWLINE {
		push1(ysasm, PUSH, (uint32_t *)&$3, sizeof(uint32_t));
	};

push_qword:
	QPUSH QQWORD QNUMBER QNEWLINE {
		push1(ysasm, PUSH, (uint64_t *)&$3, sizeof(uint64_t));
	};

push_var:
	QPUSH QTEXT QNEWLINE {
		variable_t *var = find_variable(ysasm, $2);
		add_variable_ref(ysasm, var, ysasm->ip - ysasm->prog + 1);
		push1(ysasm, PUSH, (uint64_t *)&var->addr, sizeof(uint64_t));
	};

pop:
	QPOP QNEWLINE {
		push0(ysasm, POP, sizeof(uint64_t));
	};

pop_byte:
	QPOP QBYTE QNEWLINE {
		push0(ysasm, POP, sizeof(uint8_t));
	};

pop_word:
	QPOP QWORD QNEWLINE {
		push0(ysasm, POP, sizeof(uint16_t));
	};

pop_dword:
	QPOP QDWORD QNEWLINE {
		push0(ysasm, POP, sizeof(uint32_t));
	};

pop_qword:
	QPOP QQWORD QNEWLINE {
		push0(ysasm, POP, sizeof(uint64_t));
	};

db:
	QDB QTEXT QNUMBER QNEWLINE {
		define_variable(ysasm, $2, $3, sizeof(uint8_t));
	};

dw:
	QDW QTEXT QNUMBER QNEWLINE {
		define_variable(ysasm, $2, $3, sizeof(uint16_t));
	};

dd:
	QDD QTEXT QNUMBER QNEWLINE {
		define_variable(ysasm, $2, $3, sizeof(uint32_t));
	};

dq:
	QDQ QTEXT QNUMBER QNEWLINE {
		define_variable(ysasm, $2, $3, sizeof(uint64_t));
	};

equ:
	QTEXT QEQU QNUMBER QNEWLINE {
		//define_constant(ysasm, $1, $3, sizeof(uint8_t));
	};

equrel:
	QTEXT QEQU '$' '-' QNUMBER QNEWLINE {
	};

db_chr:
	 QDB QTEXT QQUOTE QTEXT QQUOTE {
		define_variable(ysasm, $2, $4[0], sizeof(uint8_t));
	 };

db_str:
	 QDB QTEXT QDQUOTE QTEXT QDQUOTE {
		printf("2%s=%s\n", $2, $4);
	 };

load:
	QLOAD QNEWLINE {
		push0(ysasm, LOAD, sizeof(uint64_t));
	};

load_b:
	QLOAD QBYTE QNEWLINE {
		push0(ysasm, LOAD, sizeof(uint8_t));
	};

load_w:
	QLOAD QWORD QNEWLINE {
		push0(ysasm, LOAD, sizeof(uint16_t));
	};

load_d:
	QLOAD QDWORD QNEWLINE {
		push0(ysasm, LOAD, sizeof(uint32_t));
	};

load_q:
	QLOAD QQWORD QNEWLINE {
		push0(ysasm, LOAD, sizeof(uint64_t));
	};

nop:	QNOP	QNEWLINE { push0(ysasm, NOP, 	sizeof(uint8_t)); };
hlt:	QHLT	QNEWLINE { push0(ysasm, HLT, 	sizeof(uint8_t)); };
store:	QSTORE	QNEWLINE { push0(ysasm, STORE, 	sizeof(uint8_t)); };
add:	QADD	QNEWLINE { push0(ysasm, ADD, 	sizeof(uint8_t)); };
sub:	QSUB	QNEWLINE { push0(ysasm, SUB, 	sizeof(uint8_t)); };
mul:	QMUL	QNEWLINE { push0(ysasm, MUL, 	sizeof(uint8_t)); };
div:	QDIV	QNEWLINE { push0(ysasm, DIV, 	sizeof(uint8_t)); };
and:	QAND	QNEWLINE { push0(ysasm, AND, 	sizeof(uint8_t)); };
or:	QOR	QNEWLINE { push0(ysasm, OR, 	sizeof(uint8_t)); };
jmp:	QJMP	QNEWLINE { push0(ysasm, JMP, 	sizeof(uint8_t)); };
je:	QJE	QNEWLINE { push0(ysasm, JE, 	sizeof(uint8_t)); };
jz:	QJZ	QNEWLINE { push0(ysasm, JZ, 	sizeof(uint8_t)); };
jnz:	QJNZ	QNEWLINE { push0(ysasm, JNZ, 	sizeof(uint8_t)); };
call:	QCALL	QNEWLINE { push0(ysasm, CALL, 	sizeof(uint8_t)); };
dup:	QDUP	QNEWLINE { push0(ysasm, DUP, 	sizeof(uint8_t)); };
swap:	QSWAP	QNEWLINE { push0(ysasm, SWAP, 	sizeof(uint8_t)); };
int:	QINT	QNEWLINE { push0(ysasm, INT, 	sizeof(uint8_t)); };
