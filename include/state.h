/* Discrete steps for each parameter (values range from 0..STEPS) */
#define STEPS 60

/* HSV color coordinates */
extern int h, s, v;

#define LEDS 3

/* RGB duty timings */
extern float rgb[LEDS];

/* Gamma */
#define MAXCURVE 25
extern int curve;

extern int xoff, quit;
