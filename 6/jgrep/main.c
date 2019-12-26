#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define	PAGESIZE	22
char	*argv0;
char	*ps = "ps -ag";

strindex(char *s, char *t) {
	int i, n;

	n = strlen(t);
	for (i = 0; s[i]; i++)
		if (strncmp(s+i, t, n) == 0)
			return 1;
	return -1;
}

FILE *efopen(char *file, char *mode)
{
	FILE *fp;

	if ((fp = fopen(file, mode)) != NULL)
		return fp;
	fprintf(stderr, "%s: can't open file %s in mode %s\n",
			argv0, file, mode);
	exit(1);
}

int fgrep(FILE *fp, char *searchstr) {
	char buf[BUFSIZ];
	while (fgets(buf, sizeof buf, fp))
		if (strindex(buf, searchstr) >= 0)
			fputs(buf, stdout);
}

int main(int argc, char *argv[])
{
	FILE *fp;
	char *searchstr;

	argv0 = *argv++; argc--;
	if (argc == 0) {
		fprintf(stderr, "%s: please provide at least one argument.", argv0);
		exit(1);
	}

	searchstr = *argv++; argc--;

	if (argc > 0)
		while (argc--) {
			fp = efopen(*argv++, "r");
			fgrep(fp, searchstr);
		}
	else
		fgrep(stdin, searchstr);


	exit(0);
}
