PROGNAME='hoc'

$(PROGNAME): y.tab.c y.tab.h init.c math.c symbol.c code.c
	gcc y.tab.c code.c init.c math.c symbol.c -g -lm -o $(PROGNAME)

y.tab.c y.tab.h: hoc.y
	yacc -d hoc.y

clean:
	rm -rf y.tab.{c,h} $(PROGNAME)

run: $(PROGNAME)
	./$(PROGNAME)


