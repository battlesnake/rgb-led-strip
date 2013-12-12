#include "input.h"
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#include <stdio.h>

/* Test if key is waiting in buffer */
int kbhit()
{
	struct timeval tv = { 0L, 0L };
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(0, &fds);
	return select(1, &fds, NULL, NULL, &tv);
}

/* Get character (or -1/0 for error */
int readch()
{
	int r;
	unsigned char c;
	if ((r = read(0, &c, sizeof(c))) < 0) {
		return r;
	}
	else {
		return c;
	}
}

/* Get a full key code */
int readkey() {
	int c;
	/* No key */
	if (!kbhit()) {
		return 0;
	}
	c = readch();
	if (c <= 0) {
		return 0;
	}
	/* Escape code */
	if (c == ESCAPE && kbhit()) {
		c = c << 8 | readch();
		if (c == CONTROL && kbhit()) {
			c = c << 8 | readch();
		}
	}
	return c;
}

/* User input */
int input(PARSEKEY parsekey) {
	int res = 0;
	while (kbhit()) {
		res |= parsekey(readkey());
	}
	return res;
}
