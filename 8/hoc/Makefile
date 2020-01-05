PROGNAME='hoc'

$(PROGNAME): y.tab.c y.tab.h init.c math.c symbol.c
	gcc y.tab.c init.c math.c symbol.c -g -lm -o $(PROGNAME)

y.tab.c y.tab.h: hoc.y
	yacc --defines=y.tab.h hoc.y

clean:
	rm -rf y.tab.{c,h} $(PROGNAME)

run: $(PROGNAME)
	./$(PROGNAME)


