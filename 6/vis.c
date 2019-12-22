#include <stdio.h>
#include <stdlib.h>

int main() {
	int c;

	while ((c = getchar()) != EOF)
	{
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
