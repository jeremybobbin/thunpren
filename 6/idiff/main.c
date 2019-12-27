#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>

#define HUGE	100000

char	*argv0;






FILE *efopen(char *file, char *mode)
{
	FILE *fp;

	if ((fp = fopen(file, mode)) != NULL)
		return fp;
	fprintf(stderr, "%s: can't open file %s in mode %s\n",
			argv0, file, mode);
	exit(1);
}

FILE *epopen(char *cmd, char *mode)
{
	FILE *pp;

	if ((pp = popen(cmd, mode)) != NULL)
		return pp;
	fprintf(stderr, "%s: can't open pipe to '%s' in mode %s\n",
			argv0, cmd, mode);
	exit(1);
}


void nskip(FILE *fin, int n)
{
	char buf[BUFSIZ];

	while (n-- > 0)
		fgets(buf, sizeof buf, fin);
}

void ncopy(FILE *fin, int n, FILE *fout)
{
	char buf[BUFSIZ];

	while (n-- > 0) {
		if (fgets(buf, sizeof buf, fin) == NULL)
			return;
		fputs(buf, fout);
	}
}

void parse(char *s, int *pfrom1, int *pto1, int *pcmd, int *pfrom2, int *pto2)
{
#define a2i(p) while (isdigit(*s)) p = 10*(p) + *s++ - '0'
	*pfrom1 = *pto1 = *pfrom2 = *pto2 = 0;
	a2i(*pfrom1);
	if (*s == ',') {
		s++;
		a2i(*pto1);
	} else
		*pto1 = *pfrom1;

	*pcmd = *s++;
	a2i(*pfrom2);

	if (*s == ',') {
		s++;
		a2i(*pto2);
	} else
		*pto2 = *pfrom2;

}

void idiff(FILE *f1, FILE *f2, FILE *fin, FILE *fout)
{
	char tempfile[] = "idiff.XXXXXX";
	char buf[BUFSIZ], buf2[BUFSIZ];
	FILE *ft;
	char rest = '\0';
	int cmd, n, from1, to1, from2, to2, nf1, nf2;

	mktemp(tempfile);
	nf1 = nf2 = 0;

	while (fgets(buf, sizeof buf, fin) != NULL) {
		parse(buf, &from1, &to1, &cmd, &from2, &to2);
		n = to1-from1 + to2-from2 + 1;
		if (cmd == 'c')
			n += 2;
		else if (cmd == 'a')
			from1++;
		else if (cmd == 'd')
			from2++;
		printf("%s", buf);
		while (n-- > 0) {
			fgets(buf, sizeof buf, fin);
			printf("%s", buf);
		}
		do {
			printf("? ");
			fflush(stdout);

			if (rest)
				buf[0] = rest;
			else
				fgets(buf, sizeof buf, stdin);

			switch (buf[0]) {
				case '>':
					nskip(f1, to1-nf1);
					ncopy(f2, to2-nf2, fout);
					break;
				case '<':
					nskip(f2, to2-nf2);
					ncopy(f1, to1-nf1, fout);
					break;
				case 'e':
					ncopy(f1, from1-1-nf1, fout);
					nskip(f2, from2-1-nf2);
					ft = efopen(tempfile, "w");
					fprintf(ft, "---\n");
					ncopy(f2, to2+1-from2, ft);
					fclose(ft);
					sprintf(buf2, "ed %s", tempfile);
					system(buf2);
					ft = efopen(tempfile, "r");
					ncopy(ft, HUGE, fout);
					break;
				case '!':
					system(buf+1);
					printf("!\n");
					break;
				case 'q':
					if (buf[1] == '<' || buf[1] == '>') {
						rest = buf[1];
						break;
					}
					/* FALLTHROUGH */
				default:
					printf("< or > or e or !\n");
					break;
			}
		} while (buf[0] != '<' && buf[0] != '>' && buf[0] != 'e');
		nf1 = to1;
		nf2 = to2;
	}
	ncopy(f1, HUGE, fout);
	unlink(tempfile);
}

void usage() {
	fprintf(stderr, "Usage: %s file1 file2\n", argv0);
	exit(1);
}

int main(int argc, char *argv[])
{
	FILE *fin, *fout, *f1, *f2;
	char buf[BUFSIZ];
	char diffargs[BUFSIZ] = "";
	char *editor;

	argv0 = *argv;
	while (argc > 1 && argv[1][0] == '-') {
		switch (argv[1][1]) {
			case 'e': 
				if (argc < 3)
					usage();
				editor = argv[2];
				break;
			default:
				strcat(diffargs, " ");
				strcat(diffargs, argv[1]);
				break;
		}
		argc--;
		argv++;
	}
	if (argc != 3) {
		usage();
	}
	f1 = efopen(argv[1], "r");
	f2 = efopen(argv[2], "r");
	fout = efopen("idiff.out", "w");
	sprintf(buf, "diff %s %s %s", diffargs, argv[1], argv[2]);
	printf("COMMAND: %s\n",buf );

	fin = epopen(buf, "r");
	idiff(f1, f2, fin, fout);
	pclose(fin);
	printf("%s: output in file idiff.out\n", argv0);
	exit(0);
}
