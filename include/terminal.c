#include "terminal.h"
#include <memory.h>
#include <termios.h>
#include <unistd.h>

/* Original terminal state */
struct termios terminal_old_state;

void terminalinitialise()
{
	struct termios terminal_new_state;
	tcgetattr(0, &terminal_old_state);
	memcpy(&terminal_new_state, &terminal_old_state,
		sizeof(terminal_new_state));
	terminal_new_state.c_iflag |= IGNBRK | BRKINT;
	terminal_new_state.c_iflag &= ~(ISTRIP);
	terminal_new_state.c_lflag |= ISIG | IEXTEN;
	terminal_new_state.c_lflag &= ~(ICANON | ECHO | ECHONL | NL1 | CR3 | CR2 | CR1 | TAB3 | TAB2 | TAB1);
	terminal_new_state.c_oflag |= OPOST | NL0 | CR0 | TAB0;
	tcsetattr(0, TCSANOW, &terminal_new_state);
}

void terminalfinalise()
{
	tcsetattr(0, TCSANOW, &terminal_old_state);
}
