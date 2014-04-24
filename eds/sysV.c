#include <signal.h>
#ifdef LINUX
# include <slcurses.h>
#else
# include <curses.h>
#endif
/* #include <term.h> */
#include <termio.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#define		INFO_FILE_DIR	"/usr/lib/terminfo/"
#ifdef	CTRL
#undef CTRL
#endif
#define		CTRL(c)			((c)-'@')
#define		ESC				0x1b

#define	Err(m)		{ perror(m); exit(1); }

static int ctrlc = 0;
struct strs *cterm;
#if 0
SGTTY	oldtermio;      /* original tty mode */
SGTTY	newtermio;      /* cbreak/noecho tty mode */
#endif
struct winsize wsiz;

extern lastdisplay;
extern displaylen;
extern unsigned ungetdata;

void winchg(int sig)
{
	extern int displaylen;
	extern int lastdisplay;

	endwin();
	refresh();
#ifndef SLANG
	ioctl(1, TIOCGWINSZ, &wsiz);
	lastdisplay= wsiz.ws_row - 1;
	displaylen = wsiz.ws_col;
#else
	usleep(50000);
	SLtt_get_screen_size();
	lastdisplay = SLtt_Screen_Rows - 1;
	displaylen = SLtt_Screen_Cols;
#endif
	locate(1,lastdisplay + 1); rcls();
	cls();
	redisplay(sig);
	display();
	inf();
}

void ttclose()
{
	int i;
	char buf[BUFSIZ];

	/* change the terminal mode back the way it was */
	refresh();
#ifndef SLANG
	keypad(stdscr, 0);
	nl();
	cls();
	nocbreak();
	noraw();
	echo();
	endwin();
#else
	keypad(stdscr, 0);
	echo();
//	nocbreak();
	noraw();
	nl();
	SLtt_reset_video ();
	SLang_reset_tty ();
#endif
	signal(SIGINT, SIG_DFL);
}

void ctrc_c(int s)
{
	if (s == SIGINT) ctrlc = CTRL('C');
}

ttopen(void)
{
	struct sigaction sigac;

	if(initscr() == NULL) Err("Can't use CURSES ");

	/* Grab SIGWINCH (curses cannot do this correctly on its own) */
	sigaction(SIGWINCH, NULL, &sigac);
	sigaddset(&sigac.sa_mask, SIGALRM);
	sigdelset(&sigac.sa_mask, SIGINT);
	sigac.sa_handler = winchg;
	sigaction(SIGWINCH, &sigac, NULL);
	if (signal(SIGINT, ctrc_c) == SIG_ERR) Err("Unable to set signal");

	/* Set curses modes. */
#ifndef SLANG
	cbreak();
	noecho();
	keypad(stdscr, 1);
//	meta(stdscr, 1);
	nonl();
	raw();
	scrollok(stdscr, TRUE);

	ioctl(1, TIOCGWINSZ, &wsiz);
	lastdisplay= wsiz.ws_row - 1;
	displaylen = wsiz.ws_col;
#else
	SLang_init_tty (7, 0, 0);
	SLtt_get_terminfo ();
	SLtt_init_video ();
	SLtt_Use_Ansi_Colors = 0;
	noecho();
//	cbreak();
	keypad(stdscr, 1);
	nonl();
	raw();
	SLtt_get_screen_size();
	lastdisplay = SLtt_Screen_Rows - 1;
	displaylen = SLtt_Screen_Cols;
#endif

//	start_color();

	normal();
	refresh();
}

error(en, msg)
int en;
char *msg;
{
	fprintf(stderr, "Error: %s !\n", msg);
	exit(en);
}

getVkey()
{
	int c;

	c = getch();
	if (ctrlc == CTRL('C')) {
		c = CTRL('C');
		ctrlc = 0;
	}
	refresh();
	return(c);
}

keysts()
{
	int c;

	nodelay(stdscr,TRUE);
	if((c = getch()) == (-1)) return 0;
	else if (c == 0xffff) return 0;

	nodelay(stdscr,FALSE);
	refresh();
	ungetdata = c;
	return 1;
}
