#include "input.h"
#include "state.h"
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#include <stdio.h>

/* Utility: Min (int) */
int min(int a, int b)
{
	return (a > b) ? b : a;
}

/* Utility: Max (int) */
int max(int a, int b)
{
	return (a > b) ? a : b;
}

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
#define ESCAPE 0x1b
#define CONTROL ((ESCAPE << 8) | 0x5b)
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

/* Parse a key */
#define CCODE(b) ((CONTROL << 8) | b)
#define KEY_UP CCODE('A')
#define KEY_DOWN CCODE('B')
#define KEY_RIGHT CCODE('C')
#define KEY_LEFT CCODE('D')
#define KEY_PREV CCODE('5')
#define KEY_NEXT CCODE('6')
int parsekey(int c)
{
	/* Parse the code */
	switch (c) {
		/* Value */
		case KEY_UP:
			v = min(v + 1, STEPS);
			break;
		case KEY_DOWN:
			v = max(v - 1, 0);
			break;
		/* Hue */
		case KEY_LEFT:
			h = (h + STEPS - 1) % STEPS;
			break;
		case KEY_RIGHT:
			h = (h + 1) % STEPS;
			break;
		/* Saturation */
		case KEY_PREV:
			s = min(s + 1, STEPS);
			break;
		case KEY_NEXT:
			s = max(s - 1, 0);
			break;
		/* Gamma */
		case '-':
			curve = max(curve - 1, 1);
			break;
		case '+':
			curve = min(curve + 1, MAXCURVE);
			break;
		/* No output */
		case 'x':
		case 'X':
			xoff = !xoff;
			if (xoff) {
				/* Clear the screen */
				printf("\033[H\033[J");
				printf("[X]OFF");
			}
			break;
		/* Quit */
		case ESCAPE:
		case 'q':
		case 'Q':
			quit = 1;
		/* Unrecognised key */
		default:
			return 0;
	}
	return 1;
}

/* User input */
int input() {
	int res = 0;
	if (quit) {
		return -1;
	}
	while (kbhit()) {
		res |= parsekey(readkey());
	}
	return res;
}
