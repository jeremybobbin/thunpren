#include <stdio.h>
#include <stdlib.h>

int main() {
	int c, n;

	for (n = 0; (c = getchar()) != EOF; n++)
	{
		if (n == 80) {
			n = 0;
			putchar('\n');
		}
		if (!isprint(c) || c == '\\') {
			putchar('\\');
			switch (c) {
				case '\\':
					break;
				case '\t':
					c = 't';
					break;
				case '\b':
					c = 'b';
					break;
				case '\r':
					c = 'r';
					break;
				case '\n':
					c = 'n';
					break;
				case '\f':
					c = 'f';
					break;
			}
		}
		putchar(c);
	}
}
