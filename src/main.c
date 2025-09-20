/* ----- includes ----- */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>


/* ----- data ------ */

// stores terminal attributes
struct termios orig_termios;


/* ----- terminal ----- */

void die(const char* s)
{
    perror(s);
    exit(1);
}

// let the use of 'q' for exiting raw mode
void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
	die("tcsetattr");
}

void enableRawMode()
{
    // reads terminal attrs and stores in struct 'orig_termios'
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
	die("tcgetattr");
    atexit(disableRawMode);

    struct termios raw = orig_termios;

    // disables term input flags
    // IXON disables system control flow keys
    // ICRNL disables fixes the 'Ctrl-M'->10 && 'Enter'->10 to 'Ctrl-M'->13 && 'Enter'->13
    raw.c_iflag &= ~(IXON | ICRNL | ISTRIP | INPCK | BRKINT);

    // disables term output flags
    // OPOST disables '\r\n'
    raw.c_oflag &= ~(OPOST);

    // changes term control flags
    // CS8 sets the byte size to 8-bits
    raw.c_cflag |= (CS8);

    // disables term local flags
    // ECHO disables echoing feature
    // ICANON disables canonical mode
    // ISIG disables functionality of 'Ctrl-C' && 'Ctrl-Z'
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    // applies changes done
    // TCSAFLUSH does not let more input to be taken after quitting
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
	die("tcsetattr");
}


/* ----- init ----- */

int main()
{
    // disables canonical mode
    enableRawMode();


    // read() takes input and stores in c
    // pressing 'q' quits
    while (1)
    {
		char c = '\0';

		if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
	    	die("read");

		// checks if 'c' is ctrl chars
		if(iscntrl(c))
		{
	    	printf("%d\n", c);
		}
		// prints ASCII codes and char
		else
		{
	    	printf("%d ('%c')\r\n", c, c);
		}

		if ( c == 'q') break;
	}

    return 0;
}
