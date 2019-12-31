#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/time.h>
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
	exit(1);
}

long millis() {
	struct timeval tp;
	gettimeofday(&tp, (void *) NULL);
	return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}


typedef struct Line Line;
typedef struct Line {
	int offset;
	Line *next;
} Line;

int main(int argc, char *argv[])
{
	int i, count, uninit;
	FILE *fp;
	char s[BUFSIZ];
	Line *head, **this;

	argv0 = *argv;
	this = &head;
	if (argc != 2)
		die("%s: please provide a file", argv0);

	fp = efopen(argv[1], "r");
	count = i = 0;
	while ((fgets(s, sizeof(s), fp)) != NULL) {
		if (*this == NULL)
			*this = malloc(sizeof(Line));
		(*this)->offset = i;
		(*this)->next = NULL;
		this = &(*this)->next;
		i += strlen(s);
		count++;
	}
	srand(millis());
	for (i = (rand() % count); i > 0; i--)
		head = head->next;

	if (fseek(fp, (long)head->offset, SEEK_SET))
		die("%s: couldn't", argv0);

	if ((fgets(s, sizeof(s), fp)) == NULL) 
		die("%s: couldn't", argv0);

	printf("%s", s);

	exit(1);
}
