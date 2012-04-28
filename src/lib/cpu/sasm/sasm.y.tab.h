/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     QCOLON = 258,
     QNEWLINE = 259,
     QCOMMENT = 260,
     QBYTE = 261,
     QWORD = 262,
     QDWORD = 263,
     QQWORD = 264,
     QDB = 265,
     QDW = 266,
     QDD = 267,
     QDQ = 268,
     QEQU = 269,
     QNOP = 270,
     QHLT = 271,
     QLOAD = 272,
     QSTORE = 273,
     QADD = 274,
     QSUB = 275,
     QMUL = 276,
     QDIV = 277,
     QAND = 278,
     QOR = 279,
     QJMP = 280,
     QJE = 281,
     QJZ = 282,
     QJNZ = 283,
     QCALL = 284,
     QRET = 285,
     QDUP = 286,
     QPUSH = 287,
     QPOP = 288,
     QSWAP = 289,
     QINT = 290,
     QNUMBER = 291,
     QTEXT = 292
   };
#endif
/* Tokens.  */
#define QCOLON 258
#define QNEWLINE 259
#define QCOMMENT 260
#define QBYTE 261
#define QWORD 262
#define QDWORD 263
#define QQWORD 264
#define QDB 265
#define QDW 266
#define QDD 267
#define QDQ 268
#define QEQU 269
#define QNOP 270
#define QHLT 271
#define QLOAD 272
#define QSTORE 273
#define QADD 274
#define QSUB 275
#define QMUL 276
#define QDIV 277
#define QAND 278
#define QOR 279
#define QJMP 280
#define QJE 281
#define QJZ 282
#define QJNZ 283
#define QCALL 284
#define QRET 285
#define QDUP 286
#define QPUSH 287
#define QPOP 288
#define QSWAP 289
#define QINT 290
#define QNUMBER 291
#define QTEXT 292




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 27 "src/lib/cpu/sasm/sasm.y"
{
	uint64_t number;
	char *string;
}
/* Line 1529 of yacc.c.  */
#line 128 "src/lib/cpu/sasm/sasm.y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

