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

int copy(int ifd, int ofd) {
	int n;
	char buf[BUFSIZ];

	while ((n = read(ifd, buf, sizeof(buf))) > 0)
		write(ofd, buf, n);

}

typedef struct File {
	int size;
	int fd;
} File;

int main(int argc, char *argv[])
{
	struct stat buf;
	FILE *fp;
	DIR *dp;
	int i;
	File *files;
	argv0 = *argv++; argc--;

	if ((files = malloc((argc-1) * sizeof(File))) == NULL)
		exit(1);

	for (i = 0; i < argc; i++) {
		if ((files[i].fd = open(argv[i], O_RDONLY)) == -1)
			error("could not open %s", argv[i]);
		if (stat(argv[i], &buf) == -1)
			buf.st_size = 0;
		files[i].size = buf.st_size;
	}

	for (;;) {
		for (i = 0; i < argc; i++) {
			if (stat(argv[i], &buf) == -1)
				buf.st_size = 0;
			if (buf.st_size != files[i].size) {
				copy(files[i].fd, 1);
				lseek(files[i].fd, 0L, SEEK_SET);
			}
			files[i].size = buf.st_size;
		}
		sleep(2);
	}
}
