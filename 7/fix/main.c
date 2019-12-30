#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#define WORST 4
#define DIRSIZ 14

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

char ttyin(char *buf, int lim)
{
	static FILE *tty = NULL;

	if (tty == NULL)
		tty = efopen("/dev/tty", "r");
	for (;;) 
	{
		if (fgets(buf, lim, tty) == NULL || buf[0] == 'q')
			exit(0);
		else if (buf[0] == '!') {
			system(buf+1);
			printf("!\n");
			return "";
		} else 
			return buf[0];

	}
}

#define EQ(s, t) (strcmp(s, t) == 0)
int spdist(char *s, char *t) {
	int i = 0;
	/* ex before: s = "ab", t = "ac" */
	while (*s++ == *t) {
		i++;
		if (*t++ == '\0')
			return 0;
	}
	/* after: s = s+3, t = s+2 */

	if (i >= 3 && !*t) /* prefix */
		return 3;

	/* s = spaghett */
	if (*--s) {
		if (*t) {
			/* t = spahgett  */
			if (s[1] && t[1] && *s == t[1]
					&& *t == s[1] && EQ(s+2, t+2))
				return 1; /* transposition */

			/* t = spafhett  */
			if (EQ(s+1, t+1))
				return 2; /* 1 char mismatch */
		}
		/* t = spagghett  */
		if (EQ(s+1, t))
			return 2; /* extra character */
	}

	/* spahett */
	if (*t && EQ(s, t+1))
		return 2;	/* missing character */

	return WORST;
}

int mindist(char *dir, char *guess, char *best) {
	int d, nd, fd;
	struct dirent *nbuf;
	struct DIR *dp;
	if (dir[0] == '\0')
		dir = ".";
	d = WORST;
	dp = opendir(dir);
	while ((nbuf=readdir(dp)) != NULL)
		if (nbuf->d_ino) {
			nd = spdist(nbuf->d_name, guess);
			if (nd <= d && nd != WORST) {
				strcpy(best, nbuf->d_name);
				d = nd;
				if (d == 0)
					break;
			}
		}
	close(fd);
	return d;
}


int spname(char *oldname, char *newname) {
	char *p, guess[DIRSIZ+1], best[DIRSIZ+1];
	char *new = newname, *old = oldname;

	for (;;) {
		while (*old == '/')
			*new++ = *old++;
		*new = '\0';
		if (*old == '\0')
			return strcmp(oldname, newname) != 0;
		p = guess;
		for (; *old != '/' && *old != '\0'; old++)
			if (p < guess+DIRSIZ)
				*p++ = *old;
		*p = '\0';
		if (mindist(newname, guess, best) >= WORST)
			return -1; /* hopeless */

		for (p = best; *new = *p++; )
			new++;
	}
}

#define PAGESIZE 	22
int main(int argc, char *argv[])
{
	int i;
	char *p, buf[BUFSIZ], line[BUFSIZ];
	argv0 = argv[0];

	if (argc == 1) {
		fprintf(stderr, "Usage...\n");
		exit(1);
	}

	for (i = 1; i < argc; i++) {
		switch (spname(argv[i], buf)) {
			case -1:
				fprintf(stderr, "%s: could not correct \"%s\"\n", argv0, argv[i]);
				break;
			case 1:
				fprintf(stderr, "\"%s\"? ", buf);
				if (ttyin(line, BUFSIZ) == 'n')
					break;
			case 0:
				fprintf(stdout, "%s\n", buf);
				break;
		}
	}
	exit(0);
}
