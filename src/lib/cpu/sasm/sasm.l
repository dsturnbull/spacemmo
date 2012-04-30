%{
#include <stdio.h>
#include <stdlib.h>
#include "src/lib/cpu/sasm/sasm.y.tab.h"
%}

%%
byte			yylval.width = 1; return QSTYPE;
word			yylval.width = 2; return QSTYPE;
dword			yylval.width = 4; return QSTYPE;
qword			yylval.width = 8; return QSTYPE;

db			yylval.width = 1; return QDTYPE;
dw			yylval.width = 2; return QDTYPE;
dd			yylval.width = 4; return QDTYPE;
dq			yylval.width = 8; return QDTYPE;

equ			return QEQU;

nop			return QNOP;
hlt			return QHLT;
load			return QLOAD;
store			return QSTORE;
add			return QADD;
sub			return QSUB;
mul			return QMUL;
div			return QDIV;
and			return QAND;
or			return QOR;
jmp			return QJMP;
je			return QJE;
jne			return QJNE;
jz			return QJZ;
jnz			return QJNZ;
call			return QCALL;
ret			return QRET;
dup			return QDUP;
push			return QPUSH;
pop			return QPOP;
swap			return QSWAP;
int			return QINT;

[0-9]+                  yylval.number = strtoull(yytext, NULL, 0); return QNUMBER;
0x[0-9A-Fa-f]+		sscanf(yytext, "%llx", (uint64_t *)&yylval); return QNUMBER;
[_a-zA-Z][_a-zA-Z0-9]*	yylval.string = strdup(yytext); return QTEXT;
:			return QCOLON;
\n                      return QNEWLINE;
;.*			return QCOMMENT;
'			return QQUOTE;
\"			return QDQUOTE;
[ \t]+                  /* ignore whitespace */;
%%

