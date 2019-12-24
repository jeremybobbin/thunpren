#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

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

print(FILE *fp, int pagesize)
{
	static int lines = 0;
	char buf[BUFSIZ];

	while (fgets(buf, sizeof buf, fp) != NULL)
		if (++lines < pagesize)
			fputs(buf, stdout);
		else {
			buf[strlen(buf)-1] = '\0';
			fputs(buf, stdout);
			fflush(stdout);
			ttyin();
			lines = 0;
		}
			
}

int main(int argc, char *argv[])
{
	int i, pagesize = PAGESIZE;
	FILE *fp;

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
		print(stdin, PAGESIZE);
	else
		for (i = 1; i < argc; i++) {
			fp = efopen(argv[i], "r");
			print(fp, pagesize);
			fclose(fp);
		}

	exit(0);
}
