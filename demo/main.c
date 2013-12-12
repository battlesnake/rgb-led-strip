#include "main.h"
#include "state.h"
#include "pfio.h"
#include "timing.h"
#include "input.h"
#include "update.h"
#include "interface.h"
#include "control.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/mman.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

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

/* Original terminal state */
struct termios terminal_old_state;

/* Initialise the PFD interface */
void initialise()
{
	struct termios terminal_new_state;
	/* Register exit handler and signal handlers */
	atexit(&finalise);
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGQUIT, signal_handler);
	/* Open PiFace */
	controlinitialise();
	/* Initialise terminal */
	tcgetattr(0, &terminal_old_state);
	memcpy(&terminal_new_state, &terminal_old_state,
		sizeof(terminal_new_state));
	terminal_new_state.c_iflag |= IGNBRK | BRKINT;
	terminal_new_state.c_iflag &= ~(ISTRIP);
	terminal_new_state.c_lflag |= ISIG | IEXTEN;
	terminal_new_state.c_lflag &= ~(ICANON | ECHO | ECHONL | NL1 | CR3 | CR2 | CR1 | TAB3 | TAB2 | TAB1);
	terminal_new_state.c_oflag |= OPOST | NL0 | CR0 | TAB0;
	tcsetattr(0, TCSANOW, &terminal_new_state);
	/* Map timer */
	timinginitialise();
}

/* Clean up on exit */
void finalise()
{
	/* Unmap timer */
	timingfinalise();
	/* Restore terminal state */
	tcsetattr(0, TCSANOW, &terminal_old_state);
	/* Close PiFace */
	controlfinalise();
}

/* Entry point */
int main(void)
{
	initialise();
	while (!quit) {
		update();
		interface();
		while (!input()) {
			control();
		}
	}
	return 0;
}
