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
float mass = 0.0000001f;
float drag = 0.0001f;
float move_prob = 1.5f;
float scale = 0.1f;
float huestretch = 10.0f;
float well[3];
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
	srand(*timer);
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
		/* XON/XOFF */
		case 'X':
		case 'x':
			xoff = !xoff;
			if (xoff) {
				printf("\033[H\033[J[XOFF]\n");
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
void update(float dt)
{
	int i;
	/*
	 * Randomly move the well to a point such that the immediate trajectory is
	 * tangential to it
	 */
	if (ranf() > pow(1 - move_prob, dt)) {
		float vl = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		float vn[3] = { v[0] / vl, v[1] / vl, v[2] / vl };
		if (!isnan(vn[0] + vn[1] + vn[2])) {
			do {
				well[1] = (ranf() - 0.5f) / 2;
				well[2] = (ranf() - 0.5f) / 2;
				if (fabs(vn[0]) < 1e-3) {
					well[0] = 0;
				}
				else {
					well[0] = -(vn[1] * well[1] + vn[2] * well[2]) / vn[0];
				}
				for (i = 1; i < 3; i++) {
					well[i] += x[i];
				}
			} while (!(well[1] >= 0 && well[1] <= 1 && well[2] >= 0 && well[2] <= 1 && fabs(well[0]) <= 3));
			well[0] += x[0];
		}
	}
	/* Finite-difference mechanics */
	float dragfactor = pow(1 - drag, dt);
	for (i = 0; i < 3; i++) {
		float d;
		v[i] *= dragfactor;
		d = scale * (well[i] - x[i]);
		/* Prevent rapid overshoots, assume we have volumetric objects */
		if (fabs(d) > 1) {
			d = pow(d, -2);
		}
		v[i] += d * dt / mass;
		x[i] += v[i] * dt;
	}
	/* Clamping */
	hsl[0] = fmod(x[0] / huestretch + 10000, 1);
	for (i = 1; i < 3; i++) {
		if (x[i] < 0) {
			hsl[i] = 0;
			x[i] = 0;
			v[i] = fabs(v[i]) * 0.5f;
		}
		else if (x[i] > 1) {
			hsl[i] = 1;
			x[i] = 1;
			v[i] = fabs(v[i]) * -0.5f;
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
	if (xoff) {
		return;
	}
	printf(
		"\033[H\033[J"
		"HSL: \t(%.2f, %.2f, %.2f)\n"
		"X: \t(%.2f, %.2f, %.2f)\n"
		"W: \t(%.2f, %.2f, %.2f)\n"
		"\033[%d;%df\033[31;1m[*]"
		"\033[%d;%df\033[33;1m+"
		"\033[32;1m\033[4;4f+\033[4;46f+\033[26;4f+\033[26;46f+"
		"\033[0;0;0m\n",
		hsl[0], hsl[1], hsl[2],
		x[0], x[1], x[2],
		well[0], well[1], well[2],
		(int) (well[1] * 20) + 5, (int) (well[2] * 40) + 5,
		(int) (hsl[1] * 20) + 5, (int) (hsl[2] * 40) + 5);
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
	well[1] = 1.0f;
	well[2] = 0.5f;
	t2 = *timer;
	while (!quit) {
		t1 = t2;
		t2 = *timer;
		dt = (t2 - t1) * 1.0e-9f;
		if (ranf() < 0.05f)
			interface();
		input(&parsekey);
		update(dt);
		control(rgb);
	}
	return 0;
}
