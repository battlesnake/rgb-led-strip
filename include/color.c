#include "color.h"
#include <math.h>

/* Convert hsv to rgb */
void hsv2rgb(float* hsv, float* rgb)
{
	/* Convert HSV integer values to range 0..1 */
	float hue = hsv[0];
	float sat = hsv[1];
	float val = hsv[2];
	/* Sector of solor space, chroma value, and offset */
	float sector = hue * 6;
	float chroma = val * sat;
	float x = chroma * (1 - fabs(fmod(sector, 2) - 1));
	float m = val - chroma;
	/* Map values to appropriate RGB fields */
	int secnum = (int) sector;
	int ci = (secnum + 1) / 2 % 3;
	int xi = (7 - secnum) % 3;
	int zi = (5 - secnum) % 3;
	rgb[ci] = chroma + m;
	rgb[xi] = x + m;
	rgb[zi] = m;
}

/* Convert hsl to rgb */
void hsl2rgb(float* hsl, float* rgb)
{
	/* Convert HSV integer values to range 0..1 */
	float hue = hsl[0];
	float sat = hsl[1];
	float lum = hsl[2];
	/* Sector of solor space, chroma value, and offset */
	float sector = hue * 6;
	float chroma = (1 - fabs(2 * lum - 1)) * sat;
	float x = chroma * (1 - fabs(fmod(sector, 2) - 1));
	float m = lum - (chroma / 2);
	/* Map values to appropriate RGB fields */
	int secnum = (int) sector;
	int ci = (secnum + 1) / 2 % 3;
	int xi = (7 - secnum) % 3;
	int zi = (5 - secnum) % 3;
	rgb[ci] = chroma + m;
	rgb[xi] = x + m;
	rgb[zi] = m;
}
