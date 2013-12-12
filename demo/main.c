#include "main.h"
#include "util.h"
#include "format.h"
#include "color.h"
#include "piface.h"
#include "timing.h"
#include "terminal.h"
#include "input.h"
#include "control.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

/* Discrete steps for each parameter (values range from 0..STEPS) */
#define STEPS 60
/* Gamma */
#define MAXCURVE 25

/* State */
int quit = 0;
int hue = 0, sat = 0, lum = 0;
float hsl[3] = {0, 0, 0};
float rgb[3] = {0, 0, 0};
int curve = 1;
int xoff = 0;

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
}

/* Clean up on exit */
void finalise()
{
	timingfinalise();
	terminalfinalise();
	controlfinalise();
	pifacefinalise();
}

/* Parse a key */
int parsekey(int key)
{
	/* Parse the code */
	switch (key) {
		/* Value */
		case KEY_UP:
			lum = min(lum + 1, STEPS);
			break;
		case KEY_DOWN:
			lum = max(lum - 1, 0);
			break;
		/* Hue */
		case KEY_LEFT:
			hue = (hue + STEPS - 1) % STEPS;
			break;
		case KEY_RIGHT:
			hue = (hue + 1) % STEPS;
			break;
		/* Saturation */
		case KEY_PREV:
			sat = min(sat + 1, STEPS);
			break;
		case KEY_NEXT:
			sat = max(sat - 1, 0);
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
			break;
		/* Unrecognised key */
		default:
			return 0;
	}
	return 1;
}

/* Update state */
void update()
{
	hsl[0] = hue * 1.0f / STEPS;
	hsl[1] = sat * 1.0f / STEPS;
	hsl[2] = lum * 1.0f / STEPS;
	hsl2rgb(hsl, rgb);
	rgb[0] = pow(rgb[0], curve);
	rgb[1] = pow(rgb[1], curve);
	rgb[2] = pow(rgb[2], curve);
}

/* User interface */
void interface()
{
	printf(
		/* Clear the screen */
		"\033[H\033[J"
		/* HSV values */
		"Hue: %.0f%%\n"
		"Sat: %.0f%%\n"
		"Lum: %.0f%%\n"
		"\n"
		/* Gamma */
		"Gam: %d\n"
		"\n"
		/* RGB values */
		"Red: %.2f%%\n"
		"Grn: %.2f%%\n"
		"Blu: %.2f%%\n"
		"\n",
		degrees(hsl[0]), percent(hsl[1]), percent(hsl[2]),
		curve,
		percent(rgb[0]), percent(rgb[1]), percent(rgb[2]));
}

/* Entry point */
int main(void)
{
	initialise();
	while (!quit) {
		update();
		if (!xoff) {
			interface();
		}
		while (!input(&parsekey)) {
			control(rgb);
		}
	}
	return 0;
}
