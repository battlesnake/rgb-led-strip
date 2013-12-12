#include "update.h"
#include "state.h"
#include "control.h"
#include <math.h>

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
	/* Apply offset and gamma to get RGB floats */
	for (i = 0; i < LEDS; i++) {
		rgb[i] = pow(rgbf[i] + m, curve);
	}
}
