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
extern Inst *progp, *prog, *code();
#define STOP (Inst) 0

extern eval(), add(), sub(), mul(), divide(), negate(), power();
extern assign(), bltin(), varpush(), constpush(), print();
extern prexpr();
extern gt(), lt(), eq(), ge(), le(), ne(), and(), or(), not();
extern ifcode(), whilecode();

