#include "piface.h"
#include "pfio.h"

int pins[3] = {1, 0, 2};

/* PFIO initialised? */
int pfd = 0;

void pifaceinitialise()
{
	pfio_init();
	pfd = 1;
}

void pifacefinalise()
{
	if (pfd) {
		pfio_deinit();
		pfd = 0;
	}
}

/* Set the state of an LED */
void piface_power_state(int led, int state)
{
	pfio_digital_write(pins[led], state);
}
