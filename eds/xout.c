#include <slcurses.h>
#include <string.h>
#include <stdarg.h>
#include "ed.h"

#define fonly name_only

int initx = 0;
int inity = 0;
int posx = 0;
int posy = 0;
int sposx = 0;
int sposy = 0;

int fls = 0;

void cnputs(char *p)
{
	move(posy, posx);
	addstr(p);
	posx += strlen(p);
}

void cursav()
{
	sposx = posx;
	sposy = posy;
}

void curlod(void)
{
	posx = sposx;
	posy = sposy;
	move(posy, posx);
}

void locate(int x,int y)
{
	if(x) --x;
	if(y) --y;
	posx = x;
	posy = y;
	move(posy, posx);
}

void BellCall()
{
	beep();
}

void vtputs(unsigned char *p)
{
	mvaddstr(posy, posx, p);
	posx += strlen(p);
}

void vtputc(int c)
{
	mvaddch(posy, posx, c);
	++posx;
}

void hanten(void)
{
	attron(A_REVERSE);
}

void normal(void)
{
	attroff(A_STANDOUT);
}

static void  chk_bell(char *buf)
{
	char *p;

	if ((p = strchr(buf, '\a')) != NULL) {
		BellCall();
		memmove(p, p + 1, strlen(p + 1));
	}
}

void ndprintf(char *format, ...)
{
	va_list ap;
	char buf[BUFSIZ];

	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);

	chk_bell(buf);

	vtputs(buf);
}

void htprintf(char *format, ...)
{
	va_list ap;
	char buf[BUFSIZ];

	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);

	chk_bell(buf);

	hanten();
	vtputs(buf);
	normal();
}

void up(register int n)
{
	if(!n) n= 1;
	while(n--) if(posy) --posy;
	move(posy, posx);
}

void dwn(register int n)
{
	if(!n) n= 1;
	while(n--) if(posy < lastdisplay) ++posy;
	move(posy, posx);
}

void right(register int n)
{
	if(!n) n= 1;
	while(n--) if(posx < displaylen) ++posx;
	move(posy, posx);
}

void left(register short n)
{
	if(!n) n= 1;
	while(n--) if(posx) --posx;
	move(posy, posx);
}

void cls(void)
{
	clear();
	posx = posy = 0;
}

void rcls(void)
{
        int i;

	clrtoeol();
}

void curoff(void)
{
#ifdef PC98
	cnputs("\x1b[>5h");
#endif
}

void curon(void)
{
#ifdef PC98
	cnputs("\x1b[>5l");
#endif
}

unsigned char *crcut(unsigned char *p)
{
	while(*p != '\0')
		if(*p == '\n' || *p == '\r') *p= '\0';
		else ++p;
	return(p);
}

void sppcnt(int n,int htflg)
{
	register i;
	char buf[512];

	if(htflg) hanten();
	for(i =0;i < n;++i) buf[i] = ' ';
	buf[i] = '\0';
	vtputs(buf);
	if(htflg) normal();
}

void msg(char *format, ...)
{
	va_list ap;
	char buf[BUFSIZ];

	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);
	chk_bell(buf);

	posy = (ldup == 0)? lastdisplay: ldup;
	posx = 0;

	move(posy, posx);
	rcls();
	vtputs(buf);
}

void inf(void)
{
/*                     1234567890123456789  */
	static char nam[]="-  Nobby's -- ";
	int i;
	char buf[SCRN_LEN];

	nam[1]= (editfile[edfile].writeflag)? '*':'-';
	sprintf(buf,((insert)? "%s%d -- Ins -- %s --- C:%-3u L:%-4u ": "%s%d -- Rep -- %s --- C:%-3u L:%-4u ")
		, nam, edfile+1, fonly(editfile[edfile].filename), curposx+1, line->num );
	if(editfile[edfile].rdonl) strcat(buf,"-- RONLY ");
/*	locate(1,lastdisplay);	*/
	for(i=strlen(buf);i < displaylen;++i) strcat(buf,"-");
	buf[displaylen] = '\0';
	hanten();
	mvaddstr(lastdisplay - 1, 0, buf);
	normal();
}

void lineinf(void)
{
	char buf[BUFSIZ];

/*	locate(15, lastdisplay);	*/
	sprintf(buf, ((insert)? "%d -- Ins -- %s --- C:%-3u L:%-4u ": "%d -- Rep -- %s --- C:%-3u L:%-4u ")
		 , edfile+1, fonly(editfile[edfile].filename), curposx+1, line->num );
	hanten();
	mvaddstr(lastdisplay - 1, 14, buf);
	normal();
}

upscroll(int x, int y, unsigned char *buf)
{
	register i,j;

	fls = ON;
	move(y, x);
	winsertln(stdscr);
	putcrt(buf);
	msg("");
	inf();
	fls = OFF;
}

dwnscroll(int x, int y, unsigned char *buf)
{
	fls = ON;
	scroll(stdscr);
	move(y, x);
	winsertln(stdscr);
/*	clrtoeol();	*/
	putcrt(buf);
	inf();
	fls = OFF;
}
