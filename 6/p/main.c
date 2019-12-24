#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define	PAGESIZE	22
char	*argv0;

FILE *efopen(char *file, char *mode)
{
	FILE *fp;

	if ((fp = fopen(file, mode)) != NULL)
		return fp;
	fprintf(stderr, "%s: can't open file %s in mode %s\n",
			argv0, file, mode);
	exit(1);
}

char *ttyin()
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
			return buf;

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
		if (curr == NULL && ((curr = malloc(sizeof Line)) == NULL)) {
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
print(Line *head, int pagesize)
{
	Line *lptr = head;
	static int lineno;
	int target;
	lineno = target = 0;

	char *s;

	for (;;) {
		printseg(lptr, pagesize);
		s = ttyin();
		
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



int main(int argc, char *argv[])
{
	int i, pagesize = PAGESIZE;
	FILE *fp;
	Line head, *lptr;
	lptr = &head;

	head.prev = head.next = head.buf = NULL;

	argv0 = argv[0];

	if (argc > 1 && argv[1][0] == '-') {
		if ((pagesize = atoi(&argv[1][1])) < 1) {
			fprintf(stderr, "%s: pagesize cannot be less than 1\n", argv0);
			exit(1);
		}
		argc--;
		argv++;
	}

	if (argc == 1) {
		if (isatty(0)) {
			fprintf(stderr, "%s: looks like stdin is a TTY\n", argv0);
			exit(1);
		}
		ftobuf(lptr, stdin);
	}
	else
		for (i = 1; i < argc; i++) {
			fp = efopen(argv[i], "r");
			lptr = ftobuf(lptr, fp);
			fclose(fp);
		}

	print(&head, pagesize);

	exit(0);
}
