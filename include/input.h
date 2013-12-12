typedef int (*PARSEKEY)(int key);

/* Prototypes */
int input(PARSEKEY parsekey);
int kbhit();
int readch();
int readkey();
int min(int a, int b);
int max(int a, int b);

/* Some key codes */
#define ESCAPE 0x1b
#define CONTROL ((ESCAPE << 8) | 0x5b)
#define CCODE(b) ((CONTROL << 8) | b)
#define KEY_UP CCODE('A')
#define KEY_DOWN CCODE('B')
#define KEY_RIGHT CCODE('C')
#define KEY_LEFT CCODE('D')
#define KEY_PREV CCODE('5')
#define KEY_NEXT CCODE('6')
