#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define	PAGESIZE	22
char	*argv0;
char	*ps = "ps -ag";

int jstrstr(char *s, char *t, int slen, int tlen) {
	int i;
	if (tlen > slen)
		return 0;

	for (i = 0; s[i] && (slen - i) >= tlen; i++)
		if (strncmp(s+i, t, tlen) == 0)
			return 1;

	return 0;
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
	int sslen, fslen;
	char buf[BUFSIZ];
	sslen = strlen(searchstr);
	while (fgets(buf, sizeof buf, fp)) {
		fslen = strlen(buf);
		if (jstrstr(buf, searchstr, sslen, fslen))
			fputs(buf, stdout);
	}
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
