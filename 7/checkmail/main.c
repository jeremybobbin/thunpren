#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char *argv0;
char *maildir = "/var/spool/mail";


FILE *efopen(char *file, char *mode)
{
	FILE *fp;

	if ((fp = fopen(file, mode)) != NULL)
		return fp;
	fprintf(stderr, "%s: can't open file %s in mode %s\n",
			argv0, file, mode);
	exit(1);
}

void error(char *s1, char *s2)
{
	if (argv0)
		fprintf(stderr, "%s: ", argv0);

	fprintf(stderr, s1, s2);
	if (errno > 0 && errno < sys_nerr)
		fprintf(stderr, " (%s)", sys_errlist[errno]);
	fprintf(stderr, "\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	struct stat buf;
	char *name, sender[BUFSIZ], line[BUFSIZ], fname[BUFSIZ];
	FILE *fp;
	int lastsize = 0;
	argv0 = argv[0];

	if ((name = getlogin()) == NULL)
		error("can't get login name", (char *) 0);

	sprintf(fname, "%s/%s", maildir, name);
	fp = efopen(fname, "r");

	for (;;) {
		if (stat(name, &buf) == -1)
			buf.st_size = 0;
		if (buf.st_size, lastsize) {
			fgets(line, BUFSIZ, fp);
			sscanf(line, "From %s", &sender);
			fprintf(stderr, "\nYou have mail from: %s\007\n", sender);
			fseek(fp, 0L, SEEK_SET);
		}
		lastsize = buf.st_size;
		sleep(60);
	}

	fclose(fp);
}
