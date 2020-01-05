%{
#include <stdio.h>
#include "hoc.h"
extern double Pow();
double mem[26];
#define lastreg (mem['p' - 'a'])
%}
%union {
	double  val;
	Symbol  *sym;
}
%token <val>   NUMBER
%token <sym> VAR BLTIN UNDEF
%type <val>   expr asgn
%left  '+' '-'
%left  '*' '/' '%'
%left  UNARYMINUS
%left  UNARYPLUS
%%
list:
	| list '\n'
	| list ';' list
	| list asgn
	| list expr { printf("\t%.8g\n", $2); }
	| list error { yyerrok; }
	;
asgn:	VAR '=' expr { lastreg = $$=$1->u.val=$3; $1->type = VAR; }
    ;
expr:	NUMBER
	| VAR { if ($1->type == UNDEF)
			execerror("undefined variable", $1->name);
		$$ = $1->u.val; }
	| asgn
	| BLTIN '(' ')'  { $$ = (*($1->u.ptr))();}
	| BLTIN '(' expr ')'  { $$ = (*($1->u.ptr))($3);}
	| BLTIN '(' expr ',' expr ')'  { $$ = (*($1->u.ptr))($3, $5);}
	| expr '+' expr { lastreg = $$ = $1 + $3; }
	| expr '-' expr { lastreg = $$ = $1 - $3; }
	| expr '*' expr { lastreg = $$ = $1 + $3; }
	| expr '/' expr {
		if ($3 == 0.0)
			execerror("division by zero", "");
		lastreg = $$ = $1 / $3;
	}
	| expr '%' expr { lastreg = $$ = $1 - ($3 * (int)($1/$3)); }
	| '(' expr ')'  { lastreg = $$ = $2; }
	| '+' expr %prec UNARYPLUS { lastreg = $$ = $2 < 0 ? ($2 * -1) : $2; }
	| '-' expr %prec UNARYMINUS { lastreg = $$ = -$2; }
	;
%%

#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
char *argv0;
int lineno = 1;
jmp_buf begin;

void main(int argc, char **argv)
{
	argv0 = argv[0];
	init();
	setjmp(begin);
	signal(SIGFPE, fpecatch);
	yyparse();
}

void execerror(char *s, char *t)
{
	warning(s, t);
	longjmp(begin, 0);
}

void fpecatch()
{
	execerror("floating point exception", (char *) 0);
}

yylex()
{
	int c;

	while ((c=getchar()) == ' ' || c == '\t');

	if (c == EOF)
		return 0;
	if (c == '.' || isdigit(c)) { /* number */
		ungetc(c, stdin);
		scanf("%lf", &yylval.val);
		return NUMBER;
	}
	//if (islower(c)) {
	//	yylval.index = c - 'a';
	//	return VAR;
	//}
	if (isalpha(c)) {
		Symbol *s;
		char sbuf[100], *p = sbuf;
		do {
			*p++ = c;
		} while ((c=getchar()) != EOF && isalnum(c));
		ungetc(c, stdin);
		*p = '\0';
		if ((s=lookup(sbuf)) == 0)
			s = install(sbuf, UNDEF, 0.0);
		yylval.sym = s;
		return s->type == UNDEF ? VAR : s->type;
	}
	if (c == '\n')
		lineno++;
	return c;
}

void yyerror(char *s)
{
	warning(s, (char *) 0);
}

void warning(char *s, char *t)
{
	fprintf(stderr, "%s: %s", argv0, s);
	if (t)
		fprintf(stderr, " %s", t);
	fprintf(stderr, " near line %d\n", lineno);

}

