%{
#define YYSTYPE double
%}
%token NUMBER
%left  '+' '-'
%left  '*' '/' '%'
%left  UNARYMINUS
%left  UNARYPLUS
%%
list:
    | list '\n'
    | list expr '\n' { printf("\t%.8g\n", $2); }
    ;
expr:	NUMBER
    | '+' expr %prec UNARYPLUS { $$ = $2 < 0 ? ($2 * -1) : $2; }
    | '-' expr %prec UNARYMINUS { $$ = -$2; }
    | expr '+' expr { $$ = $1 + $3; }
    | expr '-' expr { $$ = $1 - $3; }
    | expr '*' expr { $$ = $1 + $3; }
    | expr '/' expr { $$ = $1 / $3; }
    | expr '%' expr { $$ = $1 - ($3 * (int)($1/$3)); }
    | '(' expr ')'  { $$ = $2; }
    ;
%%

#include <stdio.h>
#include <ctype.h>
char *argv0;
int lineno = 1;

main(int argc, char **argv)
{
	argv0 = argv[0];
	yyparse();
}

yylex()
{
	int c;

	while ((c=getchar()) == ' ' || c == '\t');

	if (c == EOF)
		return 0;
	if (c == '.' || isdigit(c)) { /* number */
		ungetc(c, stdin);
		scanf("%lf", &yylval);
		return NUMBER;
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

