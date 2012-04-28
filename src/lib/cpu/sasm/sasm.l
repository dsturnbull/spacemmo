%{
#include <stdio.h>
#include <stdlib.h>
#include "src/lib/cpu/sasm/sasm.y.tab.h"
%}

%%
byte			return QBYTE;
word			return QWORD;
dword			return QDWORD;
qword			return QQWORD;

db			return QDB;
dw			return QDW;
dd			return QDD;
dq			return QDQ;

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
[_a-zA-Z][a-zA-Z0-9]*	yylval.string = strdup(yytext); return QTEXT;
:			return QCOLON;
\n                      return QNEWLINE;
;.*			return QCOMMENT;
'			return QQUOTE;
\"			return QDQUOTE;
[ \t]+                  /* ignore whitespace */;
%%

