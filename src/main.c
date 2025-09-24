/* ----- includes ----- */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>


/* ----- defines ----- */

#define CTRL_KEY(k) ((k) & 0x1f)


/* ----- data ------ */

// Stores the global state of the editor
struct editorConfig {
	int screenrows;
	int screencols;

	struct termios orig_termios;
};

struct editorConfig E;

// stores terminal attributes
struct termios orig_termios;


/* ----- terminal ----- */

void die(const char* s)
{
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

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
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode);

    struct termios raw = E.orig_termios;

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
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

// Waits for a keypress and return it
char editorReadKey() 
{
	int nread;
	char c;
	while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
	{
		if (nread == -1 && errno != EAGAIN) die("read");
	}
	return c;
}

// Gets the window size
int getWindowSize(int *rows, int *cols)
{
	struct winsize ws;

	// Uses TIOCGWINSZ request (Terminal Input/Output/Control Get WINdow SiZe)
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
	{
		return -1;
	}
	else
	{
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
}


/* ----- output ----- */

// Draws the '~' thing like vim
// Actually called tildes
void editorDrawRows()
{
	int y;
	for (y = 0; y < E.screenrows; y++)
	{
		write(STDOUT_FILENO, "~\r\n", 3);
	}
}

// Clears the screen
void editorRefreshScreen() 
{
	// \x1b(27) is an escape character
	// [2J -> J = erase in display 
	// 	   -> 2 = clears the entire screen
	// The escape sequence is of total 4 bytes
	write(STDOUT_FILENO, "\x1b[2J", 4);
	// [H -> H = positions the cursor
	// 	  -> 1;1 = h;w
	write(STDOUT_FILENO, "\x1b[H", 3);

	editorDrawRows();

	write(STDOUT_FILENO, "\x1b[H", 3);
}


/* ----- input ----- */

// Waits for a keypress then handles it 
void editorProcessKeypress() 
{
	char c = editorReadKey();

	switch (c) 
	{
		// CTRL+q to exit
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J", 4);
			write(STDOUT_FILENO, "\x1b[H", 3);

			exit(0);
			break;
	}
}


/* ----- init ----- */

// Initializes all the fields in struct E
void initEditor()
{
	if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

int main()
{
    // disables canonical mode
    enableRawMode();
	initEditor();


    // read() takes input and stores in c
    // pressing 'q' quits
    while (1)
    {
		editorRefreshScreen();
		editorProcessKeypress();
	}

    return 0;
}
