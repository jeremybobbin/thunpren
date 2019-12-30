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

typedef struct _line {
	char *buf;
	struct _line *next;
	struct _line *prev;
} Line;

/* copies lines from fp to buf. buf is ((sizeof char *) * maxline) */
Line *ftobuf(Line *curr, FILE *fp)
{
	Line *prev = NULL;
	char line[BUFSIZ];
	int len;

	while (fgets(line, sizeof line, fp) != NULL) {
		if (curr == NULL && ((curr = malloc(sizeof(Line))) == NULL)) {
			fprintf(stderr, "Could not alloc line\n");
			exit(1);
		}
		if (prev)
		{
			curr->prev = prev;
			prev->next = curr;
		}

		curr->next = NULL;

		len = strlen(line);
		if ((curr->buf = (char *)malloc(len)) == NULL) {
			fprintf(stderr, "Could not alloc line buf\n");
			exit(1);
		}

		strcpy(curr->buf, line);

		prev = curr;
		curr = curr->next;
	}
	return prev;
}

Line *printseg(Line *lptr, int pagesize)
{
	while ((lptr = lptr->next) && pagesize--) {
		fputs(lptr->buf, stdout);
	}
	while (pagesize-- > 0)
		putc('\n', stdout);
	fflush(stdout);
	return lptr;
}

#define MIN(a, b) (a) < (b) ? (a) : (b)
#define MAX(a, b) (a) > (b) ? (a) : (b)
int print(Line *head, int pagesize)
{
	Line *lptr = head;
	static int lineno;
	int target;
	lineno = target = 0;

	char s[BUFSIZ];

	for (;;) {
		printseg(lptr, pagesize);
		ttyin(s, BUFSIZ);
		
		if (*s == 'k')
			target = MAX(lineno - pagesize, 0);
		else if (*s == 'j')
			target = lineno + pagesize;
		else if (isdigit(*s))
			target = atoi(s);
		
		while (target < lineno && lptr->prev) {
			lptr = lptr->prev;
			lineno--;
		}

		while (target > lineno && lptr->next) {
			lptr = lptr->next;
			lineno++;
		}

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
	return 4;
}

int mindist(char *dir, char *guess, char *best) {
	int d, nd, fd;
	struct dirent *nbuf;
	struct DIR *dp;
	if (dir[0] == '\0')
		dir = ".";
	d = 3;
	dp = opendir(dir);
	while ((nbuf=readdir(dp)) != NULL)
		if (nbuf->d_ino) {
			nd = spdist(nbuf->d_name, guess);
			if (nd <= d && nd != 4) {
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
		if (mindist(newname, guess, best) >= 4)
			return -1; /* hopeless */

		for (p = best; *new = *p++; )
			new++;
	}
}

#define PAGESIZE 	22
int main(int argc, char *argv[])
{
	FILE *fp;
	int i, pagesize = PAGESIZE;
	char *p, buf[BUFSIZ], line[BUFSIZ];
	Line head, *lptr;
	lptr = &head;


	argv0 = argv[0];
	head.prev = head.next = head.buf = NULL;

	if (p=getenv("PAGESIZE") != NULL)
		pagesize = atoi(p);
	if (argc > 1 && argv[1][0] == '-') {
		pagesize = atoi(&argv[1][1]);
		argc--;
		argv++;
	}
	if (argc == 1)
		print(stdin, pagesize);
	else
		for (i = 1; i < argc; i++) {
			switch (spname(argv[i], buf)) {
				case -1:
					fp = efopen(argv[i], "r");
					break;
				case 1:
					fprintf(stderr, "\"%s\"? ", buf);
					if (ttyin(line, BUFSIZ) == 'n')
						break;
					argv[i] = buf;
				case 0:
					fp = efopen(argv[i], "r");
					lptr = ftobuf(lptr, fp);
					fclose(fp);
					print(&head, pagesize);
					break;
			}
		}
	exit(0);
}
