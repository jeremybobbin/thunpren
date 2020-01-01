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
	exit(1);
}

int iocp(int ifd, int ofd) {
	int n;
	char buf[BUFSIZ];

	while ((n = read(ifd, buf, sizeof(buf))) > 0)
		if (write(ofd, buf, n) != n)
			return -1;

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
	for (i = strlen(s); s[i] != '/' && i > 0; i--);
	s[i+1] = '\0';
}

/* 
 * String *s = string_from("wow");
 * assert(s->cap = 4);
 * assert(s->len = 3);
 *
 */
typedef struct String {
	char *raw;
	int len;
	int cap; 
} String;

typedef struct String String;

/* 
 * String *s = string_new(3);
 * assert(s->cap = 4);
 */
String *string_new(int cap) {
	String *s;
	if ((s = (String *) malloc(sizeof(String))) == NULL)
		return NULL;

	s->cap = cap > 1 ? cap+1 : 1;
	if ((s->raw = (char *) malloc(sizeof (char)*(cap+1))) == NULL) {
		free(s);
		return NULL;
	}
	s->len = 0;
	s->raw[0] = '\0';
	return s;
}

String *string_from(char *cp) {
	String *s;
	int cplen;
	s = string_new(cplen = strlen(cp));
	strcpy(s->raw, cp);
	s->len = cplen;
	return s;
}

/*
 * String *s = string_from("wow")
 * assert(s->cap == 4)
 * string_grow(s, 3)
 * assert(s->cap == 4)
 *
 */
int string_grow_to(String *s, int size) {
	while (size+1 > s->cap)
		if ((s->raw = realloc(s->raw, 2*s->cap)) == NULL)
			return -1;
		else
			s->cap *= 2;

	return 0;
}

int string_grow_by(String *s, int inc)
{
	return string_grow_to(s, s->len+inc);
}


int insert_at(String *s, int index, char *cp) {
	int cplen;
	if (index > s->len+1)
		return -1;

	cplen = strlen(cp);

	/* index+1 because index starts @ 0 */
	if (string_grow_to(s, cplen + index+1) != 0)
		return -1;

	strcpy(s->raw+index, cp);
	return cplen;
}

char pop(String *s) {
	char c;
	c = s->raw[s->len];
	s->raw[s->len--] = '\0';
	return c;
}

char push(String *s, char c) {
	string_grow_by(s, 1);
	s->raw[s->len++] = c;
	s->raw[s->len] = '\0';
	return s->len;
}

void string_free(String *s) {
	free(s->raw);
	free(s);
}


void dirwalk(char *path, void (*fn)(char*))

{
	DIR *dp;
	struct dirent *dep;
	int pathlen, realloc;
	String *pathbuf;
	/* cp path to pathbuf */
	

	if ((pathbuf = string_from(path)) == NULL)
		return;
	pathlen = pathbuf->len;

	if (path[pathlen-1] == '/')
		pop(pathbuf);


	if ((dp=opendir(path)) == NULL)
		return;
	while((dep = readdir(dp)) != NULL) {
		if (dep->d_name[0] == '.')
			if (!dep->d_name[1])
				continue;
			else if (dep->d_name[1] == '.' && !dep->d_name[2]) 
				continue;

		push(pathbuf, '/');
		insert_at(pathbuf, pathlen+1, dep->d_name);
		fn(pathbuf->raw);
	}
	closedir(dp);
	string_free(pathbuf);
}

int target_ino;
char *result;

void __find_ino(char *path)
{
	struct stat stbuf;
	char target[BUFSIZ], buf[BUFSIZ];

	if (stat(path, &stbuf) == -1)
		error("can't stat file %s", path);

	if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
		dirwalk(path, __find_ino);
	} else if (stbuf.st_ino == target_ino) {
		strcpy(result, path);
		return;
	}
}


void find_ino(ino_t ino, char *path, char *found) {
	target_ino = ino;
	result = found;
	__find_ino(path);
}

int main(int argc, char *argv[])
{
	int i, fd;
	struct stat stbuf;
	char *dir = argv[argc-1];
	char path[BUFSIZ];

	argv0 = *argv;

	while (argc > 1 && argv[1][0] == '-') {
		switch (argv[1][1]) {
			default:
				  die("Unrecognized option: -%s", &argv[1][1]);
				  break;
		}
		argv++;
		argc--;
	}

	if (argc <= 2)
		die("Usage: %s files ... dir", argv0);

	find_ino(atoi(argv[1]), argv[2], path);
	if (path == NULL)
		die ("could not find %s", argv[1]);

	if ((fd = open(path, O_RDONLY)) < 0)
		die ("could not open %s", path);

	iocp(fd, 1);
	exit(0);
}
