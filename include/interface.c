#include "interface.h"
#include "state.h"
#include <stdio.h>

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
		rgb[0]*100.0f,
		rgb[1]*100.0f,
		rgb[2]*100.0f);
}
