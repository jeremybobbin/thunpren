#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int allchars(FILE *fp, int (*c)(int)) {
	while ((c = getc(fp)) != EOF)
		if (!c)
			return 0;
	return 1;
}

int isprintc(int c) {
	isprint(c);
}

int notisprintc(int c) {
	!isprint(c);
}

int main(int argc, char *argv[]) {
	int i;
	FILE *fp;
	int (*pred)(int) = isprintc;


	while (argc > 1 && argv[1][0] == '-') {
		switch (argv[1][1]) {
			case 'v':
				pred = notisprintc;
			default:
				exit(1);
		}
		argc--;
		argv++;
	}
	if (argc == 0)
		return allchars(stdin, pred);
	else 
		for (i = 1; i < argc; i++)
			if ((fp=fopen(argv[i], "r")) == NULL) {
				fprintf(stderr, "%s: can't open %s\n", argv[0], argv[i]);
				exit(1);
			} else {
				if (allchars(fp, isprintc))
					printf("%s\n", argv[i]);
			}
	return 0;
}
