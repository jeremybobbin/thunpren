#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>

char *argv0;

FILE *efopen(char *file, char *mode)
{
	FILE *fp;

	if ((fp = fopen(file, mode)) != NULL)
		return fp;
	if (errno > 0)
		fprintf(stderr, "%s: can't open file %s (%s)\n",
				argv0, file, sys_errlist[errno]);
	exit(1);
}

int main(int argc, char *argv[])
{
	FILE *fp;
	char buf[BUFSIZ];

	argv0 = argv[0];

	if (argc == 1) {
		while (fgets(buf, sizeof buf, stdin) != NULL)
			fputs(buf, stdout);

		exit(0);
	}

	while (argv++ && --argc) {
		fp = efopen(*argv, "r");
		while (fgets(buf, sizeof buf, fp) != NULL)
			fputs(buf, stdout);
	}
}
