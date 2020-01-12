#include <stdlib.h>
#include <stdio.h>
#include "hoc.h"
#include "y.tab.h"

#define NSTACK_MOD 256
static int nstack = 0;
static Datum *stack;
static Datum *stackp;

#define NPROG_MOD 2000
static int nprog = 0;
Inst *pc;
extern Inst *progp = NULL, *prog = NULL;


void initcode()
{
	if ((stackp = stack = malloc(sizeof(Datum) * NSTACK_MOD * ++nstack)) == NULL)
		execerror("could not alloc datum stack", (char *) 0);
	if ((progp = prog = malloc(sizeof(Inst) * NPROG_MOD * ++nprog)) == NULL)
		execerror("could not alloc instruction stack", (char *) 0);
}


void push(Datum d)
{
	while (stackp >= (stack+(nstack*NSTACK_MOD)))
		if ((stack = realloc(stack, sizeof(Datum) * NSTACK_MOD * ++nstack)) == NULL)
			execerror("stack overflow and no can realloc :-)", (char *) 0);

	*stackp++ = d;
}


Datum pop()
{
	if (stackp <= stack)
		execerror("stack underflow", (char *) 0);
	return *--stackp;
}

Inst *code(Inst f)
{

	Inst *oprogp = progp;
	while (progp >= (prog+(nprog*NPROG_MOD)))
		if ((prog = realloc(prog, sizeof(Inst) * NPROG_MOD * ++nprog)) == NULL)
			execerror("stack overflow and no can realloc :-)", (char *) 0);
	*progp++ = f;
	return oprogp;
}

void execute(Inst *p)
{
	for (pc = p; *pc != STOP; )
		(*(*pc++))();
}

int indent = 0;
void print_inst(Inst *pc)
{
	if (*pc == STOP)
		return;

	for (int i = indent; i > 0; i--)
		putchar('\t');

	if (*pc == whilecode) {
		printf("whilecode (\n");
		indent++;
		print_inst(&pc[3]);
		indent--;
		printf(") {\n");
		indent++;
		print_inst(pc[1]);
		indent--;
		printf("}\n");
		print_inst(pc[2]);
		return;
	} else if (*pc == ifcode) {
		printf("ifcode\n");
		return print_inst(*(pc+1));
	} else if (*pc == andcode) {
		printf("andcode\n");
		return print_inst(*(pc+1));
	} else if (*pc == orcode) {
		printf("orcode\n");
		return print_inst(*(pc+1));
	}  else if (*pc == constpush) {
		printf("constpush ");
		return debug(pc+1);
	} else if (*pc == eval) {
		printf("eval ");
		return debug(pc+1);
	} else if (*pc == varpush) {
		printf("varpush %s\n", ((Symbol *)*(pc+1))->name);
		return debug(pc+2);
	} else if (*pc == prexpr)
		printf("prexpr\n");
	else if (*pc == print)
		printf("print\n");
	else if (*pc == sub)
		printf("sub\n");
	else if (*pc == push)
		printf("push\n");
	else if (*pc == pop)
		printf("pop\n");
	else if (*pc == assign)
		printf("assign\n");
	else if (((Symbol *)*pc)->type == NUMBER) {
		printf("%lf\n", ((Symbol *)*pc)->u.val);
	} else if (((Symbol *)*pc)->type == VAR) {
		printf("number: %lf\n", ((Symbol *)*pc)->name);
	} else if (((Symbol *)*pc)->type == UNDEF) {
		printf("number: %lf\n", ((Symbol *)*pc)->name);
	} else {
		printf("\n\n");
		printf("------\n");
		printf("unknown instruction: %d\n", *pc);
		printf("type: %d\n", ((Symbol *)*pc)->type);
		printf("dvalue: %d\n", ((Symbol *)*pc)->u.val);
		printf("fvalue: %lf\n", ((Symbol *)*pc)->u.val);
		if (((Symbol *)*pc)->type == VAR) 
			printf("name: %s\n", ((Symbol *)*pc)->name);
		//printf("name: %s\n", ((Symbol *)*pc)->name);
		printf("------\n");
		printf("\n\n");
	}
	return print_inst(pc+1);
}

void debug(Inst *p)
{
	print_inst(p);
}

void constpush() 
{
	Datum d;
	d.val = ((Symbol *)*pc++)->u.val;
	push(d);
}

void varpush()
{
	Datum d;
	d.sym = (Symbol *)(*pc++);
	push(d);
}

void add()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val += d2.val;
	push(d1);
}

void sub()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val -= d2.val;
	push(d1);
}

void mul()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val *= d2.val;
	push(d1);
}

void divide()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val /= d2.val;
	push(d1);
}

void eval()
{
	Datum d;
	d = pop();
	if (d.sym->type == UNDEF)
		execerror("undefined variable",
				d.sym->name);
	d.val = d.sym->u.val;
	push(d);
}

void assign()
{
	Datum d1, d2;
	d1 = pop();
	d2 = pop();
	if (d1.sym->type != VAR && d1.sym->type != UNDEF)
		execerror("assignment to non-variable",
				d1.sym->name);
	d1.sym->u.val = d2.val;
	d1.sym->type = VAR;
	push(d2);
}


void print()
{
	Datum d;
	d = pop();
	printf("\t%.8g\n", d.val);
}

void bltin()
{
	Datum d;
	d = pop();
	d.val = (*(double (*)())(*pc++))(d.val);
	push(d);
}

void negate()
{
	Datum d;
	d = pop();
	d.val *= -1;
	push(d);
}

void gt()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val > d2.val);
	push(d1);
}

void lt()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val < d2.val);
	push(d1);
}


void eq()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val == d2.val);
	push(d1);
}

void ge()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val >= d2.val);
	push(d1);
}

void le()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val <= d2.val);
	push(d1);
}

void ne()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val != d2.val);
	push(d1);
}

void andcode()
{
	Datum d;
	Inst *savepc = pc;
	d = pop();
	if (d.val)
		execute(savepc+1);
	else
		push(d);
	pc = *((Inst **)(savepc));
}

void orcode()
{
	Datum d;
	Inst *savepc = pc;
	d = pop();
	if (d.val)
		push(d);
	else
		execute(savepc+1);
	pc = *((Inst **)(savepc));
}

void not()
{
	Datum d1 = pop();
	d1.val = (double)(!d1.val);
	push(d1);
}

void whilecode()
{
	Datum d;
	Inst *savepc = pc;

	execute(savepc+2);
	d = pop();
	while (d.val) {
		execute(*((Inst **)(savepc)));
		execute(savepc+2);
		d = pop();
	}
	pc = *((Inst **)(savepc+1));
}

void ifcode()
{
	Datum d;
	Inst *savepc = pc;
	execute(savepc+3);
	d = pop();
	if (d.val)
		execute(*((Inst **)(savepc)));
	else if (*((Inst **)(savepc+1)))
		execute(*((Inst **)(savepc+1)));
	pc = *((Inst **)(savepc+2));
}

void prexpr()
{
	Datum d;
	d = pop();
	printf("%.8g\n", d.val);
}
