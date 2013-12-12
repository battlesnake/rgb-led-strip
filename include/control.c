#include "control.h"
#include "pfio.h"
#include "timing.h"
#include <math.h>

/* Timeslice length in microseconds */
#define TIMESLICE 2000
/* Number of slices to execute in one control cycle */
#define SLICES 1

POWER_STATE power_state;

void controlinitialise(POWER_STATE pstate)
{
	power_state = pstate;
}

void controlfinalise()
{
	int i;
	if (power_state) {
		for (i = 0; i < 3; i++) {
			power_state(i, 0);
		}
	}
	power_state = 0;
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
	for (i = 0; i < 3; i++) {
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
		for (i = 0; i < 3; i++) {
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
		for (i = 0; i < 3; i++) {
			if (idx & (1 << i)) {
				power_state(i, 0);
			}
		}
	} while (idx);
}

/* Use state data to control hardware */
void control (float* rgb)
{
	int slice, i;
	int duty[3];
	/* Calculate duty timing */
	for (i = 0; i < 3; i++) {
		duty[i] = floor(rgb[i] * TIMESLICE + 0.5f);
	}
	/* Run timeslices */
	for (slice = 0; slice < SLICES; slice++) {
		timeslice(duty);
	}
}
