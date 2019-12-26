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


char ttyin()
{
	char buf[BUFSIZ];
	static FILE *tty = NULL;

	if (tty == NULL)
		tty = efopen("/dev/tty", "r");
	for (;;) 
	{
		if (fgets(buf, BUFSIZ, tty) == NULL || buf[0] == 'q')
			exit(0);
		else if (buf[0] == '!') {
			system(buf+1);
			printf("!\n");
			return "";
		} else 
			return buf[0];

	}
}

int main(int argc, char *argv[])
{
	FILE *fin;
	char buf[BUFSIZ];
	int pid;

	argv0 = argv[0];

	if ((fin = popen(ps, "r")) == NULL ) {
		fprintf(stderr, "%s: pagesize cannot be less than 1\n", argv0);
		exit(1);
	}

	fgets(buf, sizeof buf, fin);
	fprintf(stdin, "%s", buf);
	while (fgets(buf, sizeof buf, fin) != NULL) {
		if (argc == 1 || strindex(buf, argv[1]) >= 0) {
			buf[strlen(buf)-1] = '\0';
			fprintf(stderr, "%s? ", buf);
			if (ttyin() == 'y') {
				sscanf(buf, "%d", &pid);
				kill(pid, SIGKILL);
			}
		}
	}
	exit(0);
}
