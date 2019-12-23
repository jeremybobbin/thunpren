#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int isprintc(int c) {
	isprint(c);
}

int notisprintc(int c) {
	!isprint(c);
}

int allchars(FILE *fp, int (*pred)(int)) {
	int c;
	while ((c = getc(fp)) != EOF)
		if (!pred(c))
			return 0;
	return 1;
}

int allcharsprint(FILE *fp) {
	return allchars(fp, isprintc);
}

int notallcharsprint(FILE *fp) {
	return !allcharsprint(fp);
}


int main(int argc, char *argv[]) {
	int i;
	FILE *fp;
	int (*pred)(FILE *) = allcharsprint;


	while (argc > 1 && argv[1][0] == '-') {
		switch (argv[1][1]) {
			case 'v':
				pred = notallcharsprint;
				break;
			default:
				printf("unrecognized option: %c\n", argv[1][1]);
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
			} else if (pred(fp))
					printf("%s\n", argv[i]);
	return 0;
}
