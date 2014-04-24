#include <signal.h>
#include <slcurses.h>
#include <termio.h>
#include <stdlib.h>
#include <stdio.h>


struct winsize wsiz;
int lastdisplay;
int displaylen;

#if 0

void winchg(int sig)
{
	extern int displaylen;
	extern int lastdisplay;

	endwin();
	refresh();
	ioctl(1, TIOCGWINSZ, &wsiz);
	lastdisplay= wsiz.ws_row - 1;
	displaylen = wsiz.ws_col;
	locate(1,lastdisplay + 1); rcls();
	cls();
	redisplay(sig);
	display();
	inf();
}
#endif

void ttclose()
{
	int i;
	char buf[BUFSIZ];

	/* change the terminal mode back the way it was */
	refresh();
	keypad(stdscr, 0);
	nl();
	erase();
	nocbreak();
	noraw();
	echo();
	endwin();
}

ttopen()
{
	struct sigaction sigac;

	if(initscr() == NULL) {
		perror("Can't use CURSES ");
		exit(3);
	}
#if 0
	/* Grab SIGWINCH (curses cannot do this correctly on its own) */
	sigaction(SIGWINCH, NULL, &sigac);
	sigaddset(&sigac.sa_mask, SIGALRM);
	sigac.sa_handler = winchg;
	sigaction(SIGWINCH, &sigac, NULL);
#endif
	/* Set curses modes. */
	SLang_init_tty (0, 0, 0);
	SLtt_get_terminfo ();
	SLtt_init_video ();
	SLtt_Use_Ansi_Colors = 0;
	noecho();
//	cbreak();
	keypad(stdscr, 1);
	nonl();
	raw();
	cbreak();
	noecho();
	keypad(stdscr, 1);
//	meta(stdscr, 1);

//	start_color();
	ioctl(1, TIOCGWINSZ, &wsiz);
	lastdisplay= wsiz.ws_row - 1;
	displaylen = wsiz.ws_col;

}

main(int argc, char *argv[])
{
	int c;
	int l = 0;

	ttopen();
	do {
		c = getch();
		if(c == 'C') {
			clear();
			l = 0;
		}
		move(l++ ,0);
		if(l > stdscr->_maxy) l = 0;
		if(c < ' ')         printw("Keycode = 0x%02x,%03o ^%c", c, c, c + '@');
		else if( c < 0x100) printw("Keycode = 0x%02x,%03o  %c", c, c, c);
		else                printw("Keycode = 0x%02x,%03o    ", c, c);

	} while(c != 's');
	ttclose();
	return 1;
}
