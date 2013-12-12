#include "main.h"
#include "color.h"
#include "piface.h"
#include "input.h"
#include "timing.h"
#include "terminal.h"
#include "control.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <signal.h>

int quit = 0;

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

/* Initialise the PFD interface */
void initialise()
{
	/* Register exit handler and signal handlers */
	atexit(&finalise);
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGQUIT, signal_handler);
	pifaceinitialise();
	controlinitialise(&piface_power_state);
	terminalinitialise();
	timinginitialise();
	srand(time(NULL));
}

/* Clean up on exit */
void finalise()
{
	timingfinalise();
	terminalfinalise();
	controlfinalise();
	pifacefinalise();
}

/* Generate a random number in 0..1 */
float ranf()
{
	return (float) rand() / RAND_MAX;
}

/* Parse a key */
int parsekey(int key)
{
	switch (key) {
		case ESCAPE:
		case 'q':
		case 'Q':
			quit = 1;
			break;
		default:
			return 0;
	}
	return 1;
}

/* Entry point */
int main(void)
{
	initialise();
	while (!quit) {
		float hsl[3], rgb[3];
		timestamp end = markinterval() + 1000;
		hsl[0] = ranf();
		hsl[1] = 1;
		hsl[2] = ranf() > 0.75f ? 1 : 0.5f;
		hsl2rgb(hsl, rgb);
		do {
			control(rgb);
		} while (markinterval() < end);
		rgb[0] = 0;
		rgb[1] = 0;
		rgb[2] = 0;
		control(rgb);
		sleepinterval(80000, markinterval());
		input(parsekey);
	}
	return 0;
}
