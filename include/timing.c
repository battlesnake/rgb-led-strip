#include "timing.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/mman.h>

/* RasPi 1MHz clock */

volatile timestamp *timer = 0;
void *timer_base = MAP_FAILED;
int timer_fd = -1;
#define TIMER_BASE   0x20003000
#define TIMER_OFFSET 0x00000004

void timinginitialise()
{
	timer_fd = open("/dev/mem", O_RDONLY);
	if (timer_fd == -1) {
		printf("Failed to access timer device (are you root?)");
		exit(1);
	}
	timer_base = mmap(NULL, 4096, PROT_READ, MAP_SHARED, timer_fd, TIMER_BASE);
	if (timer_base == MAP_FAILED) {
		printf("Failed to access high-precision timer (are you root?)");
		exit(1);
	}
	timer = (volatile timestamp *) ((char *) timer_base + TIMER_OFFSET);
}

void timingfinalise()
{
	if (timer_base != MAP_FAILED) {
		munmap(timer_base, 4096);
		timer_base = MAP_FAILED;
	}
	if (timer_fd != -1) {
		close(timer_fd);
		timer_fd = -1;
	}
}

/* Begin interval timing */
timestamp markinterval()
{
	return *timer;
}

/* Sleep until a certain time past mark */
timestamp sleepinterval(timestamp dt, timestamp mark)
{
	return sleepuntil(mark + dt);
}

/* Sleep for a certain time, using the high precision timer */
timestamp sleepuntil(timestamp end)
{
	timestamp now = *timer;
	while (end > now) {
		now = *timer;
	}
	return now;
}

