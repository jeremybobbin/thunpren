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
#include <unistd.h>

char *argv0;
char *maildir = "/var/spool/mail";


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
	exit(1);
}

int get_uid(char *name) {
	FILE *fp;
	char buf[BUFSIZ];
	char *np, *bufp; /* name ptr, buffer ptr */

	fp = efopen("/etc/passwd", "r");
	while ((fgets(buf, BUFSIZ, fp)) != NULL) {
		np = name;
		bufp = buf;
		while (*bufp != ':' && *bufp++ == *np++);
		if (*bufp++ != ':')
			continue;
		while (*bufp++ != ':');
		fclose(fp);
		return atoi(bufp);
	}
	fclose(fp);
	return -1;
}

FILE *getusertty(char *name) {
	DIR *dp;
	FILE *fp;
	struct dirent *dentp;
	struct stat sbuf;

	int uid;
	char path[BUFSIZ];

	if ((uid = get_uid(name)) == -1) 
		return -1;

	if ((dp = opendir("/dev")) == NULL)
		error("can't open maildir", (char *) 0);

	while ((dentp = readdir(dp)) != NULL) {
		if (strncmp(dentp->d_name, "tty", 3) != 0)
			continue;

		sprintf(path, "/dev/%s", dentp->d_name);
		if (stat(path, &sbuf) == -1)
			continue;

		if (sbuf.st_uid != uid)
			continue;

		fp = efopen(path, "w");
		break;
	}
	closedir(dp);
	return fp;
}

typedef struct Mailbox Mailbox;
typedef struct Mailbox {
	char name[255];
	int uid;
	int lastsize;
	FILE *fp;
	Mailbox *next;
} Mailbox;

int main(int argc, char *argv[])
{
	struct stat buf;
	char *name, sender[BUFSIZ], line[BUFSIZ], fname[BUFSIZ];
	FILE *fp, *ttyp;
	DIR *dp;
	Mailbox *head, *mbp, **pnpp; /* prev's (next ptr) ptr */
	struct dirent *dentp;
	int lastsize = 0;
	argv0 = argv[0];

	if ((name = getlogin()) == NULL)
		error("can't get login name", (char *) 0);

	sprintf(fname, "%s/%s", maildir, name);

	if (chdir(maildir) < 0)
		error("can't change current directory to maildir", (char *) 0);
	
	if ((dp = opendir(".")) == NULL)
		error("can't open maildir", (char *) 0);

	while ((dentp = readdir(dp)) != NULL) {
		if (*dentp->d_name == '.')
			continue;
		if (stat(dentp->d_name, &buf) == -1)
			continue;
		pnpp = &head;
		while (*pnpp) {
			pnpp = &(*pnpp)->next;
		}

		*pnpp = malloc(sizeof(Mailbox));
		(*pnpp)->fp = efopen(dentp->d_name, "r");

		strcpy((*pnpp)->name, dentp->d_name);
		(*pnpp)->uid = buf.st_uid;
		(*pnpp)->lastsize = buf.st_size;
		(*pnpp)->next = NULL;
	}

	for (;;) {
		for (mbp = head; mbp; mbp = mbp->next) {
			if (stat(mbp->name, &buf) == -1)
				buf.st_size = 0;
			if (buf.st_size > mbp->lastsize) {
				fgets(line, BUFSIZ, mbp->fp);
				sscanf(line, "From %s", &sender);
				ttyp = getusertty(mbp->name);
				fprintf(ttyp, "\nYou have mail from: %s\007\n", sender);
				fclose(ttyp);
				fseek(mbp->fp, 0L, SEEK_SET);
			}
			mbp->lastsize = buf.st_size;
		}
		sleep(2);
	}

	fclose(fp);
}
