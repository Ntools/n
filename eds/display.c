#include "ed.h"
#ifndef UNIX
#include <jctype.h>
#endif

int lng= EOF;
int refr_flg= 0;
unsigned char extchar[80];
int curfile= 0;
SCRN *opline;

extern tabset;

#ifdef UNIX

#define fonly       name_only

unsigned char *name_only(unsigned char *nam)
{
	unsigned char *p;

	p = nam;
	while(*nam) if(*nam++ == '/') p = nam;
	return(p);
}
#endif

void outopen(unsigned char *name)
{
	unsigned char *p,buf[80];
	extern FILE *out_fp;

	strcpy(buf,name);
	if((p= strchr(buf,'.')) != NULL) *p= '\0';
	strcat(buf,".bak");
	out_fp= fopen(buf,"w");
}

unsigned char *charscrn(int c,int len)
{
	static int i = 0;
	static int kanji= OFF;
	static unsigned char buf[256];

	if(c == '\n') {
		if(kanji) buf[i-1]= ' ';
		buf[i]= '\0';
		
		vtputs(buf);
		kanji = i= 0;
	}
	else if(len >= rightshift && i < displaylen) {
		if(kanji) kanji= OFF;
		else if(iskanji(c)) kanji = ON;
		buf[i++]= c;
	}
	return(buf);
}

unsigned char *scrnset(unsigned char *p)
{
	int i,line_len;

	line_len= 0;
	while(*p != '\0') {
		if(*p == '\t') {
			i= (line_len % tabs);
			while(i < tabs) {
				charscrn(' ',line_len++);
				++i;
			}
		}
		else if(*p == '\n') break;
		else if(*p > 0 && *p < ' ') {
			charscrn('^',line_len++);
			charscrn((*p+'@'),line_len++);
		}
		else charscrn(*p, line_len++);
		++p;
		if((line_len-rightshift) >= displaylen) 
			break;
	}
	return(charscrn('\0',rightshift));
}

unsigned char *putcrt(unsigned char *p)
{
	scrnset(p);
	return(charscrn('\n',rightshift));      /* buffer flush */
}

void display(void)
{
	int  i;
	SCRN *lp;
	extern SCRN *smlptr;

	if(!refr_flg) lp= distop;
	else lp= smlptr;
#if (defined TEXT98)
	curoff();
#endif
	for(i= refr_flg+1+widbase;i < lastdisplay;++i) {
		locate(1,i);
		rcls();
		if(lp != NULL) {
			putcrt(lp->buffer);
			lp= lp->fwd;
		}
		else charscrn('\n',rightshift);
	}
	inf();
	refr_flg= 0;
#if (defined TEXT98)
	curon();
#endif
	locate(poschk(),curposy+1);
}

#ifndef UNIX 
# ifndef __386__
void msg(unsigned char *fmt,...)
{ 
	va_list argptr;
	unsigned char buf[BUFSIZ];
	int y,x;

	va_start(argptr,fmt);
	vsprintf(buf,fmt,argptr);
	va_end(argptr);
	crcut(buf);
	y= (ldup == 0)? lastdisplay+1: ldup+1;
	locate(1,y);
	rcls();
	buf[79]= '\0';
	x= strlen(buf);
	vtputs(buf);
	locate(x,y);
}
# endif

void inf(void)
{
/*                     1234567890123456789   */
	static unsigned char nam[]="-  Nobby's -- ";
	int i;
	unsigned char buf[SCRN_LEN];

	nam[1]= (editfile[edfile].writeflag)? '*':'-';
	sprintf(buf,((insert)? "%s%d -- Ins -- %s --- C:%-3u L:%-4u ": "%s%d -- Rep -- %s --- C:%-3u L:%-4u ")
	  , nam, edfile+1, fonly(editfile[edfile].filename), curposx+1, line->num );
	if(editfile[edfile].rdonl) strcat(buf,"-- RONLY ");
	locate(1,lastdisplay);
	for(i=strlen(buf);i < displaylen;++i) strcat(buf,"-");
	buf[displaylen] = '\0';
	htprintf("%s",buf);
}

void lineinf(void)
{
	locate(15, lastdisplay);
	htprintf(((insert)? "%d -- Ins -- %s --- C:%-3u L:%-4u ": "%d -- Rep -- %s --- C:%-3u L:%-4u ")
	, edfile+1, fonly(editfile[edfile].filename), curposx+1, line->num );
}

#endif


void tablenset()
{
unsigned char *p;

	if((p= strrchr(editfile[edfile].filename,'.')) == NULL) {
		lng= EOF; extchar[0]= '\0';
		if(tabset == OFF) tabs= tabtbl[OTHLNG];
		return;
	}
	strcpy(extchar,p);
	if(strcmpi(p,".c") == 0) {
		lng = C;
		if(tabset == OFF) tabs= tabtbl[C];
	}
	else if(strcmpi(p,".h") == 0) {
		lng= EOF;
		if(tabset == OFF) tabs= tabtbl[C];
	}
	else if(strcmpi(p,".a") == 0) {
		lng= ASM;
		if(tabset == OFF) tabs= tabtbl[ASM];
	}
	else if(strcmpi(p,".asm") == 0) {
		lng= MASM;
		if(tabset == OFF) tabs= tabtbl[MASM];
	}
	else {
		lng= EOF;
		if(tabset == OFF) tabs= tabtbl[OTHLNG];
	}
}

windowop(void)
{
	if(ldup == 0) {
		widbase= lastdisplay / 2;
		refr_flg= 0;
		display();
		ldup= lastdisplay;
		lastdisplay /= 2;
		widbase= 0;
		opline= distop;
		curfile= edfile;
		curposy= disset(3);
		refr_flg= last= OFF;
		return(OK);
	}
	else return(NG);
}

windowcl(void)
{
	if(ldup) {
		line= distop;
		curposx= 0;
		lastdisplay= ldup;
		curposy= widbase= ldup= refr_flg= last= OFF;
		return(OK);
	}
	else return(NG);
}

chgwindow(void)
{
	int swp;
	SCRN *lp;

	if(ldup) {
		lp= distop;
		line= distop= opline;
		opline= lp;

		swp= edfile;
		edfile= curfile;
		curfile= swp;
		if(widbase) {
			curposy= widbase= 0;
			lastdisplay= ldup / 2;
		}
		else {
			curposy= widbase= (ldup / 2);
			lastdisplay= ldup;
		}
		return(OK);
	}
	return(NG);
}

readonly(void)
{
	editfile[edfile].rdonl ^= ON;
	inf();
	return(OK);
}

void helpfile(void)
{
	int i;
	unsigned cu,cl;
	unsigned char buf[80],buf1[40];
	SCRN *lp;
	extern back_flg;
	extern FILE *out_fp;
	void tablenset(void),funcbuffer();

	for(i=0;i < MAXFILE && editfile[i].topline != NULL;++i) ;
	edfile= i;
	editfile[edfile].filename= "keys.hlp";
	editfile[edfile].rdonl = ON;
	tablenset();
	if((lp= memgetl(NULL)) == NULL) return;
	line_num= 0;
	line= lp;
	editfile[edfile].writeflag= OFF;

	strcpy(buf,"KeyWord „Ÿ„Ÿ„Ÿ KeyCode „Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ Function „Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ„Ÿ");
	lp->num= ++line_num;
	if((lp->buffer= memgets(buf)) == NULL) return;
	if((lp= memgetl(lp)) == NULL) return;
	for(i= 0;keyptr[i].keyword != NULL;++i) {
		if(*keyptr[i].keyword == '\0') continue;
		if(strlen(keyptr[i].keyword) < 8) sprintf(buf,"%s\t\t",keyptr[i].keyword);
		else sprintf(buf,"%s\t",keyptr[i].keyword);
		cu= keyptr[i].code & 0xff00;
		cl= keyptr[i].code & 0xff;
		buf1[0] = '\0';
#if (defined TEXT98) || (defined DOSV)
		if(cu == FUNC_CODE)
			funcbuffer(keyptr[i].code,buf);
		else {
#endif
			if(cu == META) strcat(buf,"META ");
			else if(cu == CTRX) strcat(buf,"EXTEND ");
			if(cl <= ' ') {
				if(cl == 27) sprintf(buf1,"ESC");
				else sprintf(buf1,"^%c",(cl+(unsigned)'@'));
			}
			else if(cl < 0x7f) sprintf(buf1,"%c",cl);
			else if(cl == 0x7f) sprintf(buf1,"DEL");
			else sprintf(buf1,"0x%02x",cl);
#if (defined TEXT98) || (defined DOSV)
		}
#endif
		strcat(buf, buf1);
		strcat(buf,(logical(buf,strlen(buf)) < 24) ? "\t\t\t":"\t\t");
		strcat(buf,keyptr[i].helpmsg);
		lp->num= ++line_num;
		if((lp->buffer= memgets(buf)) == NULL) break;
		if((lp= memgetl(lp)) == NULL) break;
	}
	lp->buffer= memgets(""); lp->num= ++line_num;
	curposx= 0; curposy= widbase; last= OFF;
	editfile[edfile].curline= editfile[edfile].topline= distop= line;
	mark.line= line;
	mark.posx= 0;
}


readfile(void)
{
	unsigned char *p;
	unsigned char buf[BUFSIZ];
	SCRN *lp;
	extern back_flg;
	extern FILE *out_fp;
	void tablenset(void);

	if(editfile[edfile].writeflag) {
		if(ynchk("Discard chages") != YES) return(NG);
		clearmem(editfile[edfile].topline,EOF);
	}

	tablenset();
	line_num= 0;
	editfile[edfile].writeflag= OFF;
	if((in_fp= fopen(editfile[edfile].filename,"r")) == NULL) {
		if((lp= memgetl(NULL)) == NULL) return(NG);
		lp->buffer = memgets("");
		msg("New File [%s]\n",editfile[edfile].filename);
	}
	else {
		if(back_flg) outopen(editfile[edfile].filename);
		lp = NULL;
		while(fgets(buf,BUFSIZ,in_fp) != NULL) {
			if(out_fp != NULL) fprintf(out_fp,"%s",buf);
			crcut(buf);
			if((lp= memgetl(lp)) == NULL) break;
			lp->num= ++line_num;
			if((lp->buffer= memgets(buf)) == NULL) break;
		}
		fclose(in_fp);
		if(out_fp != NULL) fclose(out_fp);
		out_fp= NULL;
	}
	curposx= 0; curposy= widbase; last= OFF;
	editfile[edfile].curline= editfile[edfile].topline= distop= line;
	mark.line= line;
	mark.posx= 0;
	return(NG);
}
