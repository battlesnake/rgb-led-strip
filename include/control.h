typedef void (*POWER_STATE)(int led, int state);

/* Prototypes */
void controlinitialise(POWER_STATE pstate);
void controlfinalise();
void control(float* rgb);
void timeslice(int *rgb);
