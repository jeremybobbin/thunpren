int yylex();
void execerror(char *s, char *t);
void fpecatch();
void yyerror(char *s);
void warning(char *s, char *t);


typedef struct Symbol {
	char *name;
	short type;
	union {
		double  val;
		double  (*ptr)();
		int     (*defn)();
		char    *str;
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
extern Inst *progp, *prog, *progbase, *code();
#define STOP (Inst) 0

void execute(Inst *p);
extern void eval(), add(), sub(), mul(), divide(), negate(), power();
extern void assign(), bltin(), varpush(), constpush(), print(), varread();
extern void prexpr(), prstr();
extern void gt(), lt(), eq(), ge(), le(), ne(), and(), or(), not();
extern void ifcode(), whilecode(), call(), arg(), argassign();
extern void funcret(), procret();

