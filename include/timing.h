/* Timer data type */
typedef unsigned long long int timestamp;

/* Prototypes */
void timinginitialise();
void timingfinalise();
timestamp markinterval();
timestamp sleepinterval(timestamp dt, timestamp mark);
timestamp sleepuntil(timestamp end);

extern volatile timestamp *timer;
