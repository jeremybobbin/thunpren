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

Symbol *install(), *lookup();
