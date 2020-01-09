#include <stdlib.h>
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


initcode()
{
	if ((stackp = stack = malloc(sizeof(Datum) * NSTACK_MOD * ++nstack)) == NULL)
		execerror("could not alloc datum stack", (char *) 0);
	if ((progp = prog = malloc(sizeof(Inst) * NPROG_MOD * ++nprog)) == NULL)
		execerror("could not alloc instruction stack", (char *) 0);
}


push(Datum d)
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

execute(Inst p)
{
	for (pc = p; *pc != STOP; )
		(*(*pc++))();
}

constpush() 
{
	Datum d;
	d.val = ((Symbol *)*pc++)->u.val;
	push(d);
}

varpush()
{
	Datum d;
	d.sym = (Symbol *)(*pc++);
	push(d);
}

add()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val += d2.val;
	push(d1);
}

sub()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val -= d2.val;
	push(d1);
}

mul()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val *= d2.val;
	push(d1);
}

divide()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val /= d2.val;
	push(d1);
}

eval()
{
	Datum d;
	d = pop();
	if (d.sym->type == UNDEF)
		execerror("undefined variable",
				d.sym->name);
	d.val = d.sym->u.val;
	push(d);
}

assign()
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


print()
{
	Datum d;
	d = pop();
	printf("\t%.8g\n", d.val);
}

bltin()
{
	Datum d;
	d = pop();
	d.val = (*(double (*)())(*pc++))(d.val);
	push(d);
}

negate()
{
	Datum d;
	d = pop();
	d.val *= -1;
	push(d);
}

gt()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val > d2.val);
	push(d1);
}

lt()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val < d2.val);
	push(d1);
}


eq()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val == d2.val);
	push(d1);
}

ge()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val >= d2.val);
	push(d1);
}

le()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val <= d2.val);
}

ne()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val != d2.val);
	push(d1);
}

and()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val && d2.val);
	push(d1);
}

or()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (double)(d1.val || d2.val);
	push(d1);
}

not()
{
	Datum d1 = pop();
	d1.val = (double)(!d1.val);
	push(d1);
}

whilecode()
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

ifcode()
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

prexpr()
{
	Datum d;
	d = pop();
	printf("%.8g\n", d.val);
}
