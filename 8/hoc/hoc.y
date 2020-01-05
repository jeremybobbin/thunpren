%{
double mem[26];
%}
%union {
	double  val;
	int     index;
}
%token <val>   NUMBER
%token <index> VAR
%type <val>   expr
%left  '+' '-'
%left  '*' '/' '%'
%left  UNARYMINUS
%left  UNARYPLUS
%%
list:
	| list '\n'
	| list expr '\n' { printf("\t%.8g\n", $2); }
	| list error '\n' { yyerrok; }
	;
expr:	NUMBER
	| VAR { $$ = mem[$1]; }
	| VAR '=' expr { $$ = mem[$1] = $3; }
	| expr '+' expr { $$ = $1 + $3; }
	| expr '-' expr { $$ = $1 - $3; }
	| expr '*' expr { $$ = $1 + $3; }
	| expr '/' expr {
		if ($3 == 0.0)
			execerror("division by zero", "");
		$$ = $1 / $3;
	}
	| expr '%' expr { $$ = $1 - ($3 * (int)($1/$3)); }
	| '(' expr ')'  { $$ = $2; }
	| '+' expr %prec UNARYPLUS { $$ = $2 < 0 ? ($2 * -1) : $2; }
	| '-' expr %prec UNARYMINUS { $$ = -$2; }
	;
%%

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
char *argv0;
int lineno = 1;
int fpecatch();
jmp_buf begin;

main(int argc, char **argv)
{
	argv0 = argv[0];
	setjmp(begin);
	signal(SIGFPE, fpecatch);
	yyparse();
}

execerror(char *s, char *t)
{
	warning(s, t);
	longjmp(begin, 0);
}

fpecatch()
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
	if (islower(c)) {
		yylval.index = c - 'a';
		return VAR;
	}
	if (c == '\n')
		lineno++;
	return c;
}

yyerror(char *s)
{
	warning(s, (char *) 0);
}

warning(char *s, char *t)
{
	fprintf(stderr, "%s: %s", argv0, s);
	if (t)
		fprintf(stderr, " %s", t);
	fprintf(stderr, " near line %d\n", lineno);

}

