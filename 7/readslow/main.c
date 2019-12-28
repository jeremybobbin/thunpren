#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	char buf[BUFSIZ];
	int n, time;

	if (argc == 3 && strcmp(argv[1], "-n"))
		time = atoi(argv[2]);

	for (;;) {
		while ((n = read(0, buf, sizeof buf)) > 0)
			write(1, buf, n);
		sleep(time);
	}
}
