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
#include <fcntl.h>
#include <unistd.h>

char *argv0;

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

int copy(int ifd, int ofd) {
	int n;
	char buf[BUFSIZ];

	while ((n = read(ifd, buf, sizeof(buf))) > 0)
		if (write(ofd, buf, n) != n)
			return -1;

}

void sv(char *file, char *dir)
{
	struct stat sti, sto;
	int fin, fout, n;

	char target[BUFSIZ], buf[BUFSIZ];

	sprintf(target, "%s/%s", dir, file);

	if (index(file, '/') != NULL)
		error("won't handle /'s in %s", file);
	if (stat(file, &sti) == -1)
		error("can't stat %s", file);
	if (stat(target, &sto) == -1)
		sto.st_mtime = 0;

	if (sti.st_mtime < sto.st_mtime)
		fprintf(stderr, "%s: %s not copied\n", argv0, file);
	else if ((fin = open(file, 0)) == -1)
		error("can't open file %s", file);
	else if ((fout = creat(target, sti.st_mode)) == -1)
		error("can't create %s", target);
	else if (copy(fin, fout) == -1)
		error("error writing %s", target);

	close(fin);
	close(fout);
}

int main(int argc, char *argv[])
{
	int i;
	struct stat stbuf;
	char *dir = argv[argc-1];

	argv0 = *argv;
	if (argc <= 2)
		error("Usage: %s files ... dir", argv0);
	if (stat(dir, &stbuf) == -1)
		error("can't access dirctory %s", dir);
	for (i = 1; i < argc; i++)
		sv(argv[i], dir);

	exit(1);

}
