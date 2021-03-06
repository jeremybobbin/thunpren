%{
#include "hoc.h"
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#define code2(c1, c2)     code(c1); code(c2)
#define code3(c1, c2, c3) code(c1); code(c2); code(c3);
char     *argv0;      
int      lineno    =  1;
jmp_buf  begin;       
int      indef;       
char     *infile;     
FILE     *fin;        
char     **gargv;     
int      gargc;       
int      c;

%}
%union {
	Symbol  *sym;
	Inst    *inst;
	int     narg;
}
%token <sym> NUMBER STRING PRINT VAR BLTIN UNDEF WHILE IF ELSE
%token <sym> FUNCTION PROCEDURE RETURN FUNC PROC READ
%token <narg> ARG
%type <inst> expr stmt asgn prlist stmtlist
%type <inst> cond while if begin end
%type <sym> procname
%type <narg> arglist argdecllist
%right '='
%left  OR
%left  AND
%left  GT GE LT LE EQ NE
%left  '+' '-'
%left  '*' '/' '%'
%left  UNARYMINUS UNARYPLUS NOT
%left '^'
%%
list:
	| list '\n'
	| list defn '\n'
	| list asgn '\n' { code2(pop, STOP); return 1; }
	| list stmt '\n' { code(STOP); return 1; }
	| list expr '\n' { code2(print, STOP); return 1; }
	| list error '\n' { yyerrok; }
	;
asgn:	  VAR '=' expr { code3(varpush, (Inst)$1, assign); $$=$3; }
	| ARG '=' expr
		{ defnonly("$"); code2(argassign, (Inst)$1); $$=$3; }
	;
stmt:	  expr { code(pop); }
	| RETURN { defnonly("return"); code(procret); }
	| RETURN expr
		{ defnonly("return"); $$=$2; code(funcret); }

	| PROCEDURE begin '(' arglist ')'
		{ $$=$2; code3(call, (Inst)$1, (Inst)$4); }
	| PRINT prlist { $$ = $2; }
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
end:	{ code(STOP); $$ = progp; }
   	;
stmtlist:	{ $$ = progp; }
	| stmtlist '\n'
	| stmtlist stmt
   	;
expr:	  NUMBER { $$ = code2(constpush, (Inst)$1); }
	| VAR    { $$ = code3(varpush, (Inst)$1, eval); }
	| ARG    { defnonly("$"); $$ = code2(arg, (Inst)$1); }
	| asgn
	| FUNCTION begin '(' arglist ')'
		{ $$ = $2; code3(call, (Inst)$1, (Inst)$4); }
	| READ '(' VAR ')' { $$ = code2(varread, (Inst)$3); }
	| BLTIN '(' expr ')' 
		{ $$ = $3; code2(bltin, (Inst)$1->u.ptr); }
	| '(' expr ')' { $$ = $2; }
	| expr '+' expr { code(add); }
	| expr '-' expr { code(sub); }
	| expr '*' expr { code(mul); }
	| expr '/' expr { code(divide); }
	| '-' expr %prec UNARYMINUS { code(negate); }
	| expr GT expr { code(gt); }
	| expr GE expr { code(ge); }
	| expr LT expr { code(lt); }
	| expr LE expr { code(le); }
	| expr EQ expr { code(eq); }
	| expr NE expr { code(ne); }
	| expr AND expr { code(and); }
	| expr OR expr { code(or); }
	| NOT expr { $$ = $2; code(not); }
	;
begin:	{ $$ = progp; }
	;
prlist:   expr { code(prexpr); }
	| STRING { $$ = code2(prstr, (Inst)$1); }
	| prlist ',' expr { code(prexpr); }
	| prlist ',' STRING { code2(prstr, (Inst)$3); }
	;
defn:     FUNC procname { $2->type=FUNCTION; indef=1; }
		'(' argdecllist ')' stmt { code(funcret); define($2); indef=0; } 
	| PROC procname { $2->type=PROCEDURE; indef=1; }
		'(' argdecllist ')' stmt { code(procret); define($2); indef=0; }
	;
procname: VAR
	| FUNCTION
	| PROCEDURE
	;
argdecllist: { $$ = 0; }
	| VAR { code2(arg, (Inst)1); code3(varpush, (Inst)$1, assign);  $$ = 1;  }
	| argdecllist ',' VAR { code2(arg, (Inst)($1 + 1)); code3(varpush, (Inst)$3, assign); $$ = ($1 + 1); }
	;
arglist:  { $$ = 0; }
	| expr { $$ = 1; }
	| arglist ',' expr { $$ = $1 + 1; }
	;
%%
int moreinput()
{
	if (gargc-- <= 0)
		return 0;
	if (fin && fin != stdin)
		fclose(fin);
	infile = *gargv++;
	lineno = 1;
	if (strcmp(infile, "-") == 0) {
		fin = stdin;
		infile = 0;
	} else if ((fin=fopen(infile, "r")) == NULL) {
		fprintf(stderr, "%s: can't open %s\n", argv0, infile);
		return moreinput();
	}
	return 1;
}

run()
{
	setjmp(begin);
	signal(SIGFPE, fpecatch);
	for (initcode(); yyparse(); initcode())
		execute(progbase);
}

int main(int argc, char **argv)
{
	extern Inst *prog;
	argv0 = argv[0];
	if (argc == 1) {
		static char *stdinonly[] = { "-" };
		gargv = stdinonly;
		gargc = 1;
	} else {
		gargv = argv+1;
		gargc = argc-1;
	}
	init();
	while (moreinput())
		run();
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

void defnonly(char *s)
{
	if (!indef)
		execerror(s, "used outside definition");
}

int follow(expect, ifyes, ifno)
{
	int c = getchar();

	if (c == expect)
		return ifyes;
	ungetc(c, stdin);
	return ifno;
}

int backslash(int c)
{
	char *index();
	static char transtab[] = "b\bf\fn\nr\rt\t";
	if (c != '\\')
		return c;
	c = getc(fin);
	if(islower(c) && index(transtab, c))
		return index(transtab, c)[1];
	return c;
}

int yylex()
{
	int c;

	while ((c=getchar()) == ' ' || c == '\t');

	if (c == EOF)
		return 0;
	
	if (c == '$') {
		int n = 0;
		while (isdigit(c=getc(fin)))
			n = 10 * n + c - '0';
		ungetc(c, fin);
		if (n == 0)
			execerror("strange $...", (char *)0);
		yylval.narg = n;
		return ARG;
	}

	if (c == '"') {
		char sbuf[100], *p, *emalloc();
		for (p = sbuf; (c=getc(fin)) != '"'; p++) {
			if (c == '\n' || c == EOF)
				execerror("missing quote", "");
			if (p >= sbuf + sizeof(sbuf) - 1) {
				*p = '\0';
				execerror("string too long", sbuf);
			}
			*p = backslash(c);
		}
		*p = 0;
		yylval.sym = (Symbol *)emalloc(strlen(sbuf)+1);
		return STRING;
	}



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
		case '=': return follow('=', EQ, '=');
		case '!': return follow('=', NE, NOT);
		case '|': return follow('|', OR, '|');
		case '&': return follow('&', AND, '&');
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


