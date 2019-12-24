#include <stdio.h>
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

ttyin()
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
int ftobuf(Line *curr, FILE *fp)
{
	Line *prev;
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
}

print(Line *head, int pagesize)
{
	static int lineno = 0;
	Line *lptr;
	char *buf;

	for (lptr = head; lptr; lptr = lptr->next) {
		buf = lptr->buf;
		if (((++lineno) % pagesize) != 0) {
			fputs(buf, stdout);
		} else {
			buf[strlen(buf)-1] = '\0';
			fputs(buf, stdout);
			fflush(stdout);
			ttyin();
		}
	}
}

int main(int argc, char *argv[])
{
	int i, pagesize = PAGESIZE;
	FILE *fp;
	Line lines;

	argv0 = argv[0];

	if (argc > 1 && argv[1][0] == '-') {
		if ((pagesize = atoi(&argv[1][1])) < 1) {
			fprintf(stderr, "%s: pagesize cannot be less than 1\n", argv0);
			exit(1);
		}
		argc--;
		argv++;
	}

	if (argc == 1)
	{
		ftobuf(&lines, stdin);
		fprintf(stderr, "%s: got %s\n", argv0, lines.buf);
		fprintf(stderr, "%s: got %s\n", argv0, lines.next->buf);
		print(&lines, PAGESIZE);
	}
	//else
	//	for (i = 1; i < argc; i++) {
	//		fp = efopen(argv[i], "r");
	//		print(fp, pagesize);
	//		fclose(fp);
	//	}

	exit(0);
}
