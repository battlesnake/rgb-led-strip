#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/mman.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include "pfio.h"

/* Discrete steps for each parameter (values range from 0..STEPS) */
#define STEPS 60
/* Timeslice length in microseconds */
#define TIMESLICE 2000
/* Number of slices to execute in one control cycle */
#define SLICES 10
/* LED pin numbers */
#define LEDS 3
int pins[LEDS] = {1, 0, 2};

/* Timer data type */
typedef unsigned long long int timestamp;

/* Prototypes */
void initialise();
void finalise();
void signal_handler(int signal);

int input();
int kbhit();
int readch();
int readkey();
int parsekey(int c);
int min(int a, int b);
int max(int a, int b);

void interface();
int degrees(int num);
int percent(int num);

void update();

void control();
void timeslice();
void power_state(int led, int state);
timestamp markinterval();
timestamp sleepinterval(timestamp dt, timestamp mark);
timestamp sleepuntil(timestamp end);

int main(void);

/* State information */
int h = 0, s = 0, v = 0;
int rgb[LEDS] = {0, 0, 0};
#define MAXCURVE 25
int curve = 1;
int xoff = 0, quit = 0;

/* Signal handler */
void signal_handler(int signal)
{
	switch (signal) {
		case SIGINT:
		case SIGTERM:
		case SIGQUIT:
			quit = 1;
			break;
	}
}

/* RasPi 1MHz clock */
volatile timestamp *timer = 0;
void *timer_base = MAP_FAILED;
int timer_fd = -1;
#define TIMER_BASE   0x20003000
#define TIMER_OFFSET 0x00000004

/* Original terminal state */
struct termios terminal_old_state;

/* PFD initialised */
int pfd = 0;

/* Initialise the PFD interface */
void initialise()
{
	struct termios terminal_new_state;
	/* Register exit handler */
	atexit(&finalise);
	/* Register signal handler */
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGQUIT, signal_handler);
	/* Open PiFace */
	pfio_init();
	pfd = 1;
	/* Initialise terminal */
	tcgetattr(0, &terminal_old_state);
	memcpy(&terminal_new_state, &terminal_old_state,
		sizeof(terminal_new_state));
	terminal_new_state.c_iflag |= IGNBRK | BRKINT;
	terminal_new_state.c_iflag &= ~(ISTRIP);
	terminal_new_state.c_lflag |= ISIG | IEXTEN;
	terminal_new_state.c_lflag &= ~(ICANON | ECHO | ECHONL | NL1 | CR3 | CR2 | CR1 | TAB3 | TAB2 | TAB1);
	terminal_new_state.c_oflag |= OPOST | NL0 | CR0 | TAB0;
	tcsetattr(0, TCSANOW, &terminal_new_state);
	/* Map timer */
	timer_fd = open("/dev/mem", O_RDONLY);
	if (timer_fd == -1) {
		printf("Failed to access timer device (are you root?)");
		exit(1);
	}
	timer_base = mmap(NULL, 4096, PROT_READ, MAP_SHARED, timer_fd, TIMER_BASE);
	if (timer_base == MAP_FAILED) {
		printf("Failed to access high-precision timer (are you root?)");
		exit(1);
	}
	timer = (volatile timestamp *) ((char *) timer_base + TIMER_OFFSET);
}

/* Clean up on exit */
void finalise()
{
	/* Unmap timer */
	if (timer_base != MAP_FAILED) {
		munmap(timer_base, 4096);
		timer_base = MAP_FAILED;
	}
	if (timer_fd != -1) {
		close(timer_fd);
		timer_fd = -1;
	}
	/* Restore terminal state */
	tcsetattr(0, TCSANOW, &terminal_old_state);
	/* Close PiFace */
	if (pfd) {
		int i;
		for (i = 0; i < LEDS; i++) {
			power_state(i, 0);
		}
		pfio_deinit();
		pfd = 0;
	}
}

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

/* Format to percentage */
int percent(int num)
{
	return (num * 100) / STEPS;
}

/* Format to degrees */
int degrees(int num)
{
	return (num * 360) / STEPS;
}

/* User interface */
void interface()
{
	if (xoff) {
		return;
	}
	printf(
		/* Clear the screen */
		"\033[H\033[J"
		/* HSV values */
		"Hue: %d%%\n"
		"Sat: %d%%\n"
		"Val: %d%%\n"
		"\n"
		/* Gamma */
		"Gam: %d\n"
		"\n"
		/* RGB values */
		"Red: %.2f%%\n"
		"Grn: %.2f%%\n"
		"Blu: %.2f%%\n"
		"\n",
		degrees(h), percent(s), percent(v),
		curve,
		rgb[0]*100.0f/TIMESLICE,
		rgb[1]*100.0f/TIMESLICE,
		rgb[2]*100.0f/TIMESLICE);
}

/* Update the control state */
void update()
{
	/* Convert HSV integer values to range 0..1 */
	float hue = h * 1.0f / STEPS;
	float sat = s * 1.0f / STEPS;
	float val = v * 1.0f / STEPS;
	/* Sector of solor space, chroma value, and offset */
	float sector = hue * 6;
	float chroma = val * sat;
	float x = chroma * (1 - fabs(fmod(sector, 2) - 1));
	float m = val - chroma;
	/* Map values to appropriate RGB fields */
	int secnum = (int) sector;
	int ci = (secnum + 1) / 2 % 3;
	int xi = (7 - secnum) % 3;
	float rgbf[LEDS] = {0, 0, 0};
	int i;
	rgbf[ci] = chroma;
	rgbf[xi] = x;
	/* Apply offset to get RGB and convert to duty timing */
	for (i = 0; i < LEDS; i++) {
		rgb[i] = floor(pow(rgbf[i] + m, curve) * TIMESLICE + 0.5f);
	}
}

/* Begin interval timing */
timestamp markinterval()
{
	return *timer;
}

/* Sleep until a certain time past mark */
timestamp sleepinterval(timestamp dt, timestamp mark)
{
	sleepuntil(mark + dt);
}

/* Sleep for a certain time, using the high precision timer */
#define WASTE_LEN 1000
int waste[WASTE_LEN];
timestamp sleepuntil(timestamp end)
{
	timestamp now = *timer;
	while (end > now) {
		/*
		 * Using the timer too rapidly prevents the loop from ever ending for
		 * some strange reason that I'd love to know.  Xeno's paradox?
		 */
		int i;
		for (i = 1; i < WASTE_LEN; i++)
			waste[i] -= waste[i - 1];
		now = *timer;
	}
	return now;
}

/* Set the state of an LED */
void power_state(int led, int state)
{
	pfio_digital_write(pins[led], state);
}

/* Executes one timeslice */
void timeslice() {
	int i;
	int t1, t2, t;
	int idx;
	timestamp mark;
	/*
	 * Enable all pins with positive duty before the timeslice begins, but
	 * disable those which are at zero duty
	 */
	for (i = 0; i < LEDS; i++) {
		power_state(i, rgb[i] > 0);
	}
	/* Sleep until action is needed, then take action (i.e. disable pins) */
	t2 = 0;
	mark = markinterval();
	do {
		/*
		 * Build a bitmask of next pin(s) which need to be disabled, and the
		 * time until they need to be disabled
		 */
		idx = 0;
		t1 = t2;
		t2 = TIMESLICE;
		for (i = 0; i < LEDS; i++) {
			int t = rgb[i];
			if (t > t1 && t <= t2) {
				if (t == t2) {
					idx |= 1 << i;
				}
				else {
					idx = 1 << i;
					t2 = t;
				}
			}
		}
		/* Sleep until action is needed */
		sleepinterval(t2, mark);
		/* Disable masked pins */
		for (i = 0; i < LEDS; i++) {
			if (idx & (1 << i)) {
				power_state(i, 0);
			}
		}
	} while (!quit && idx);
}

/* Use state data to control hardware */
void control ()
{
	int slice;
	for (slice = 0; slice < SLICES; slice++) {
		timeslice();
	}
}

/* Entry point */
int main(void)
{
	initialise();
	while (!quit) {
		update();
		interface();
		while (!input()) {
			control();
		}
	}
	return 0;
}
