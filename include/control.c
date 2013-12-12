#include "control.h"
#include "state.h"
#include "pfio.h"
#include "timing.h"
#include <math.h>

/* Timeslice length in microseconds */
#define TIMESLICE 2000
/* Number of slices to execute in one control cycle */
#define SLICES 1
int pins[LEDS] = {1, 0, 2};

/* PFIO initialised? */
int pfd = 0;

void controlinitialise()
{
	pfio_init();
	pfd = 1;
}

void controlfinalise()
{
	if (pfd) {
		int i;
		for (i = 0; i < LEDS; i++) {
			power_state(i, 0);
		}
		pfio_deinit();
		pfd = 0;
	}
}

/* Set the state of an LED */
void power_state(int led, int state)
{
	pfio_digital_write(pins[led], state);
}

/* Executes one timeslice */
void timeslice(int *rgb) {
	int i;
	int t1, t2;
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
	int slice, i;
	int duty[LEDS];
	/* Calculate duty timing */
	for (i = 0; i < LEDS; i++) {
		duty[i] = floor(rgb[i] * TIMESLICE + 0.5f);
	}
	/* Run timeslices */
	for (slice = 0; slice < SLICES; slice++) {
		timeslice(duty);
	}
}
