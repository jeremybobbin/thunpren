PROGNAME='hoc'

run: $(PROGNAME)
	./$(PROGNAME)


$(PROGNAME): y.tab.c
	gcc y.tab.c -g -lm -o $(PROGNAME)

y.tab.c: hoc.y
	yacc hoc.y
