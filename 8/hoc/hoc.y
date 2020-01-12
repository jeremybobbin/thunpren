%{
#include "hoc.h"
extern double Pow();
double mem[26];
extern Inst *prog;
#define code2(c1, c2)     code(c1); code(c2)
#define code3(c1, c2, c3) code(c1); code(c2); code(c3)
#define code7(c1, c2, c3, c4, c5, c6, c7) code(c1); code(c2); code(c3); code(c4); code(c5); code(c6); code(c7)
%}
%union {
	Symbol  *sym;
	Inst    *inst;
}
%token <sym> NUMBER PRINT VAR BLTIN UNDEF WHILE IF ELSE AND OR
%type <inst> stmt asgn expr stmtlist cond while if end and or
%right  AEQ SEQ
%right MEQ DEQ
%right ASSIGN
%left  OR
%left  AND
%left  GT GE LT LE EQ NE
%left  ADD SUB
%left  MUL DIV
%left  UNARYMINUS UNARYPLUS NOT
%left '^'
%%
list:
	| list '\n'
	| list ';' list
	| list asgn { code2(pop, STOP); return 1; }
	| list stmt { code(STOP); return 1; }
	| list expr { code2(print, STOP); return 1; }
	| list error { yyerrok; }
	;
asgn:     VAR ASSIGN expr { code3(varpush, (Inst)$1, assign); }
	| VAR AEQ expr { code7(varpush, (Inst)$1, eval, add, varpush, (Inst)$1, assign); }
	| VAR SEQ expr { code7(varpush, (Inst)$1, eval, sub, varpush, (Inst)$1, assign); }
	| VAR MEQ expr { code7(varpush, (Inst)$1, eval, mul, varpush, (Inst)$1, assign); }
	| VAR DEQ expr { code7(varpush, (Inst)$1, eval, div, varpush, (Inst)$1, assign); }
	;
stmt:	  expr { code(pop); }
	| PRINT expr { code(prexpr); $$ = $2; }
	| while cond stmt end {
		($1)[1] = (Inst)$3;
		($1)[2] = (Inst)$4; }
	| if cond stmt end {
		($1)[1] = (Inst)$3;
		($1)[3] = (Inst)$4; }
	| if cond stmt end ELSE stmt end {
		($1)[1] = (Inst)$3;
		($1)[2] = (Inst)$6;
		($1)[3] = (Inst)$7; }
	| '{' stmtlist '}' { $$ = $2; }
	;
cond:	'(' expr ')' { code(STOP); $$ = $2; }
    	;
while:	WHILE { $$ = code3(whilecode, STOP, STOP); }
    	;
if:	IF { $$=code(ifcode); code3(STOP, STOP, STOP); }
  	;
and:	AND { $$ = code3(andcode, STOP, STOP); }
    	;
or:	OR { $$ = code3(orcode, STOP, STOP); }
    	;
end:	{ code(STOP); $$ = progp; }
   	;
stmtlist:	{ $$ = progp; }
	| stmtlist '\n'
	| stmtlist stmt
   	;
expr:	  NUMBER { code2(constpush, (Inst)$1); }
	| VAR    { code3(varpush, (Inst)$1, eval); }
	| asgn
	| BLTIN '(' expr ')' 
		{ $$ = $3; code2(bltin, (Inst)$1->u.ptr); }
	| '(' expr ')' { $$ = $2; }
	| expr ADD expr { code(add); }
	| expr SUB expr { code(sub); }
	| expr MUL expr { code(mul); }
	| expr DIV expr { code(divide); }
	| '-' expr %prec UNARYMINUS { code(negate); }
	| expr GT expr { code(gt); }
	| expr GE expr { code(ge); }
	| expr LT expr { code(lt); }
	| expr LE expr { code(le); }
	| expr EQ expr { code(eq); }
	| expr NE expr { code(ne); }
	| expr and expr end {
		($2)[1] = (Inst)$3;
		($2)[2] = (Inst)$4; }
	| expr or expr end {
		($2)[1] = (Inst)$3;
		($2)[2] = (Inst)$4; }
	//| expr AND expr { code(and); }
	//| expr OR expr { code(or); }
	| NOT expr { code(not); }
	;
%%
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
char *argv0;
int lineno = 1;
jmp_buf begin;

int main(int argc, char **argv)
{
	extern Inst *prog;
	argv0 = argv[0];
	init();
	setjmp(begin);
	signal(SIGFPE, fpecatch);
	if (argc > 1 && strcmp(argv[1], "-d") == 0)
		for (initcode(); yyparse(); initcode())
			debug(prog);
	else
		for (initcode(); yyparse(); initcode())
			execute(prog);
	return 0;
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

int follow(expect, ifyes, ifno)
{
	int c = getchar();

	if (c == expect)
		return ifyes;
	ungetc(c, stdin);
	return ifno;
}

int yylex()
{
	int c;

	while ((c=getchar()) == ' ' || c == '\t');

	if (c == EOF)
		return 0;
	if (c == '.' || isdigit(c)) { /* number */
		double d;
		ungetc(c, stdin);
		scanf("%lf", &d);
		yylval.sym = install("", NUMBER, d);
		return NUMBER;
	}
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
	switch (c) {
		case '>': return follow('=', GE, GT);
		case '<': return follow('=', LE, LT);
		case '=': return follow('=', EQ, ASSIGN);
		case '!': return follow('=', NE, NOT);
		case '|': return follow('|', OR, '|');
		case '&': return follow('&', AND, '&');

		case '+': return follow('=', AEQ, ADD);
		case '-': return follow('=', SEQ, SUB);
		case '*': return follow('=', MEQ, MUL);
		case '/': return follow('=', DEQ, DIV);

		case '\n': lineno++;
		default: return c;
	}
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

