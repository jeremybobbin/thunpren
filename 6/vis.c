#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int readable(int c) {
	return isascii(c) && (isprint(c) || c=='\n' || c=='\t' || c==' ');
}

int main(int argc, char *argv[]) {

	int c, i = 0, strip = 0, enough = 0;
	char *s;

	if (argc >= 3 && strcmp(argv[1], "-s") == 0)
		strip = atoi(argv[2]) ;

	if ((s = malloc(strip)) == NULL)
		exit(1);

	while ((c = getchar()) != EOF) {
		if (readable(c)) {
			if (i < strip)
				s[i++] = c;
			else {
				for (i = 0; i < strip; i++)
					putchar(s[i]);
				i = 0;
				do {
					putchar(c);
				} while ((c = getchar()) != EOF && readable(c));

				putchar('\n');
			}

		} else if (!strip)
			printf("\\%03o", c);
		else
			i = 0;
	}


	exit(0);
}
