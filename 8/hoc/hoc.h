void execerror(char *s, char *t);
yylex();
void fpecatch();
void yyerror(char *s);
void warning(char *s, char *t);

typedef struct Symbol {
	char *name;
	short type;
	union {
		double val;
		double (*ptr)();
	} u;
	struct Symbol *next;
} Symbol;

typedef struct Datum {
	double val;
	Symbol *sym;
} Datum;
extern Datum pop();
Symbol *install(), *lookup();

typedef int (*Inst)();
#define STOP (Inst) 0

extern Inst *prog;
extern eval(), add(), sub(), mul(), divide(), negate(), power();
extern assign(), bltin(), varpush(), constpush(), print();
