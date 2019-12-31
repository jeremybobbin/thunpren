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
int vflag = 0;

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
}

void die(char *s1, char *s2)
{
	if (argv0)
		fprintf(stderr, "%s: ", argv0);

	fprintf(stderr, s1, s2);
	if (errno > 0 && errno < sys_nerr)
		fprintf(stderr, " (%s)", sys_errlist[errno]);
	fprintf(stderr, "\n");
}

int iocp(int ifd, int ofd) {
	int n;
	char buf[BUFSIZ];

	while ((n = read(ifd, buf, sizeof(buf))) > 0)
		if (write(ofd, buf, n) != n)
			return -1;

}

void dirwalk(char *from, char *to, void (*fn)(char *, char*))
{
	DIR *dp;
	struct dirent *dep;
	struct stat stbuf;
	char pathbuf[BUFSIZ];

	if ((dp=opendir(from)) == NULL)
		return;
	while((dep = readdir(dp)) != NULL) {
		if (dep->d_name[0] == '.' && !dep->d_name[1] ||
				strcmp(dep->d_name, "..") == 0)
			continue;

		sprintf(pathbuf, "%s/%s", from, dep->d_name);
		fn(pathbuf, to);
	}

}

char *basename(char *s)
{
	int i;
	/*  sets i to last '/' */
	for (i = strlen(s); s[i] != '/' && i >= 0; i--);
	return s+i+1;
}

void dirname(char *s)
{
	int i;
	for (i = strlen(s); s[i] != '/' && i >= 0; i--);
	s[i] = '\0';
}

void cp(char *file, char *dir)
{
	struct stat sti, sto;
	int fin, fout, n;

	char target[BUFSIZ], buf[BUFSIZ];

	if (stat(dir, &sto) == -1)
		die("can't stat target %s", dir);

	if ((sto.st_mode & S_IFMT) == S_IFDIR)
		sprintf(target, "%s/%s", dir, basename(file));
	else
		strcpy(target, dir);

	//if (index(file, '/') != NULL) {
	//	error("won't handle /'s in %s", file);
	//	return;
	//}
	if (stat(file, &sti) == -1) {
		error("can't stat %s", file);
		return;
	}
	if (stat(target, &sto) == -1)
		sto.st_mtime = 0;



	if (sti.st_mtime < sto.st_mtime && vflag) {
		fprintf(stderr, "%s: %s not copied\n", argv0, file);
		return;
	} else if ((sti.st_mode & S_IFMT) == S_IFDIR) {
		mkdir(target, sti.st_mode);
		dirwalk(file, target, cp);
		return;
	} else if ((fin = open(file, 0)) == -1) {
		error("can't open file %s", file);
		return;
	} else if ((fout = creat(target, sti.st_mode)) == -1) {
		error("can't create %s", target);
		return;
	} else if (iocp(fin, fout) == -1) {
		error("error writing %s", target);
		return;
	}

	close(fin);
	close(fout);
}

int main(int argc, char *argv[])
{
	int i;
	struct stat stbuf;
	char *dir = argv[argc-1];

	argv0 = *argv;

	while (argc > 1 && argv[1][0] == '-') {
		switch (argv[1][1]) {
			case 'v':
				vflag = 1;
				break;
			default:
				  die("Unrecognized option: -%s", &argv[1][1]);
				  break;
		}
		argv++;
		argc--;
	}

	if (argc <= 2)
		die("Usage: %s files ... dir", argv0);
	for (i = 1; i < argc-1; i++)
		cp(argv[i], dir);

	exit(1);

}
