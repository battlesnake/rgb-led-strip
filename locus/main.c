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
#include <time.h>
#include <signal.h>

/* State */
int quit = 0;
float hsl[3] = {0, 0, 0};
float rgb[3] = {0, 0, 0};
float x[3] = {0, 0, 0};
float v[3] = {0, 0, 0};
float mass = 6000.0f;
float drag = 0.35f;
float well[3];

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

/* Parse a key */
int parsekey(int key)
{
	/* Parse the code */
	switch (key) {
		/* Value */
		case KEY_UP:
			mass = min(mass + 1, 5);
			break;
		case KEY_DOWN:
			mass = max(mass - 1, 1);
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
void update(float dt)
{
	int i;
	if (ranf() > pow(0.7f, dt)) {
		well[0] = x[0] + ((ranf() - 0.5f) / 2);
		well[1] = pow(ranf(), 0.5f);
		well[2] = pow(ranf() - 0.5f, 3) + 0.5f;
	}
	float dragfactor = pow(1 - drag, dt);
	for (i = 0; i < 3; i++) {
		v[i] *= dragfactor;
		v[i] += (well[i] - x[i]) * dt / mass;
		x[i] += v[i];
	}
	hsl[0] = fmod(x[0] + 1, 1);
	for (i = 1; i < 3; i++) {
		if (x[i] < 0) {
			hsl[i] = 0;
			v[i] = 0;
			x[i] = 0;
		}
		else if (x[i] > 1) {
			hsl[i] = 1;
			v[i] = 0;
			x[i] = 1;
		}
		else {
			hsl[i] = x[i];
		}
	}
	hsl2rgb(hsl, rgb);
}

/* Random number */
float ranf()
{
	return (float) rand() / RAND_MAX;
}

void interface()
{
	printf(
		"\033[H\033[J"
		"HSL: \t(%.2f, %.2f, %.2f)\n"
		"X: \t(%.2f, %.2f, %.2f)\n"
		"Well: \t(%.2f, %.2f, %.2f)\n",
		hsl[0], hsl[1], hsl[2],
		x[0], x[1], x[2],
		well[0], well[1], well[2]);
}

/* Entry point */
int main(void)
{
	timestamp t1, t2;
	float dt;
	initialise();
	x[0] = 100 + ranf();
	x[1] = 0;
	x[2] = 0;
	v[0] = 0;
	v[1] = 0;
	v[2] = 0;
	well[0] = x[0];
	well[1] = ranf();
	well[2] = 0.5f;
	t1 = *timer;
	while (!quit) {
		t2 = *timer;
		dt = (t2 - t1) * 1.0e-9f;
		//if (ranf() < 0.01f)
		//	interface();
		input(&parsekey);
		update(dt);
		control(rgb);
	}
	return 0;
}
