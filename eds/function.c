/**************************************

       Commands execute functions

**************************************/
#include "ed.h"
#ifndef UNIX
#include <jctype.h>
#elif defined(LINUX)
#include <slcurses.h>
#else
#include <curses.h>
#endif

#ifdef TEXT98

#if (defined _MSC_VER)
#include    <memory.h>
#endif

#define TEXTSTART               ((void _far *)0xa0000000)
#define ATTRSTART              ((void _far *)0xa2000000)

unsigned short *text_vram = TEXTSTART;
unsigned short *attr_vram = ATTRSTART;
#endif

#if (defined ATVRAM)
extern void __far *vram;
void vtflush(void __far *, size_t );
#endif

extern refr_flg;
extern lng;
int posflag = OFF;
int cnv_err = OFF;
int kfind = OFF;
int sc_cnt = 0;
int res_wd = OFF;
int coment = OFF;

typedef struct
{
 unsigned char*wd;
       int cnt;
} RES_WD;

RES_WD c_rev_wd[] = {
  
	{"while", 0},
	{"for",2},
	{"if",0},
	{"else",0},
	{"do", 0},
	{NULL, 0}
};

res_find(unsigned char*p)
{
	int i;

	for(i = 0;c_rev_wd[i].wd != NULL;++i)
		if(strcmp(p, c_rev_wd[i].wd) == 0) return(i);
	return OFF;
}

SCRN *nextlp(SCRN *lp)
{
	do {
		lp = lp->fwd;
		if(lp == NULL) return(NULL);
	} while(lp->buffer[0] == '#') ;
	return(lp);
}

SCRN *backlp(SCRN *lp, int i)
{
	int cmt;

	if(i) --i;
	cmt = OFF;
	while(lp != NULL) {
		if(lp->buffer[0] != '#') {
			while(i > 0) {
				if(lp->buffer[i - 1] == '/') {
					if(lp->buffer[i] == '*') {
						cmt = ON;
						if(i <= 1) break;
						i -= 2;
					}
					else if(lp->buffer[i] == '/') {
						lp = lp->rev;
						cmt = OFF;
						break;
					}
					else --i;
				}
				else if(cmt && lp->buffer[i] == '/' && lp->buffer[i - 1] == '*') {
					cmt = OFF;
					if(i <= 1) break;
					i -= 2;
				}
				else if(!cmt && lp->buffer[i] > ' ') return(lp);
				else --i;
			}
		}
		if(lp->rev == NULL) break;
		lp = lp->rev;
		i = strlen(lp->buffer);
	}
	return(lp);
}

SCRN *startk(void)
{
	SCRN *lp;

	lp = line->rev;
	while(lp != NULL) {
		if(lp->buffer[0] == '{') break;
		else if(lp->buffer[0] > '@' && strchr(lp->buffer,'{') != NULL) break;
		lp = lp->rev;
	}
	return(lp);
}

int ksearch(int c, SCRN *cp)
{
	SCRN *lp;
	int ce, qt, kc, wd;
	unsigned char*p, buf[10];

	res_wd = sc_cnt = kfind = kc = qt = coment = wd = 0;
	if(c == '{') ce = '}';
	else if(c =='(') ce = ')';
	else ce = ']';
	if((lp = startk()) == NULL) return 0;
	p = lp->buffer;
	while(p != &line->buffer[curposx]) {
		if(!coment && !qt && *p == c) {
			++kc;
			if(c == '{' && cp == lp) kfind = ON;
		}
		else if(!coment && !qt && *p == ce) --kc;
		else if(!coment && !qt && *p == ';') {
			if(cp == lp) ++sc_cnt;
		}
		else if(*p == '\0') {
			if((lp = nextlp(lp)) == NULL) break;
			p = lp->buffer;
			continue;
		}
		else if(*p == '\"') {
			if(qt == '\"') qt = 0;
			else if(qt == 0) qt = '\"';
			else ;
		}
		else if(*p == '\'') {
			if(p[1] == '\\' && p[3] == '\'') p = &p[3];
			else if(p[2] == '\'') p = &p[2];
			else if(qt == '\'') qt = 0;
			else if(qt == 0) qt = '\'';
			else ;
		}
		else if(*p == '/') {
			if(p[1] == '*') coment = ON;
			else if(p[1] == '/') {
				if(lp == line) {
					coment = ON;
					break;
				}
				else if((lp = nextlp(lp)) == NULL) break;
				p = lp->buffer;
				continue;
			}
		}
		else if(coment && *p == '*' && p[1] == '/') coment = OFF;
		else if(*p == '\\') {
			if(p[1] == 'x' || p[1] == 'X') {
				do {
					++p;
				} while(isxdigit(*p)) ;
			}
			else if(isdigit(*p)) 
				while(isdigit(*p)) ++p;
			else p += 2;
			continue;
		}
		else if(!coment && !qt && lp == cp) {
			if(*p >= 'a' && *p <= 'z') buf[wd++] = *p;
			else {
				buf[wd] = '\0';
				if((wd = res_find(buf)) != OFF) res_wd = wd;
				wd = 0;
			}
		}
		++p;
	}
	if(kc < 0) kc = 0;
	return(kc);
}

int k1count(SCRN *lp)               /* for '{' '}' */
{
	k1cnt = ksearch('{', lp);
	if(!kfind && res_wd) {
		if(c_rev_wd[res_wd].cnt >= sc_cnt) ++k1cnt;
	}
	return(k1cnt);
}

int charactor(unsigned short c)
{
unsigned char buf[BUFSIZ+1];
register int i;
int kflg;

	if(curposx >= BUFSIZ) return(NG);
	strcpy(buf,line->buffer);
	kflg= OFF;
	do {
		if(insert) {
			for(i= curposx;buf[i] != '\0';++i) ;
			while(i >= curposx) {
				buf[i+1]= buf[i];
				--i;
			}
		}
		else
		if(buf[curposx] == '\0') buf[curposx+1]= '\0';
		buf[curposx++]= c;
		if(!kflg && iskanji(c)) {
			kflg= ON;
			c = getkey();
		}
		else kflg = OFF;
	} while(kflg);
	return(update(buf,0));
}

int delchar(void)
{
	unsigned char buf[BUFSIZ],*p;
	register int i;

	if(line->buffer[curposx]) {
		p=
#ifdef KNJ
		   (iskanji(line->buffer[curposx]) && iskanji2(line->buffer[curposx+1]))? &line->buffer[curposx+2]:
#endif
		   &line->buffer[curposx+1];
		line->buffer[curposx]= '\0';
		sprintf(buf,"%s%s",line->buffer,p);
	}
	else {
		if(line->fwd != NULL &&
			(strlen(line->fwd->buffer)+strlen(line->buffer)) < (BUFSIZ-1)) {
			sprintf(buf,"\b%s%s",line->buffer,line->fwd->buffer);
			line= line->fwd; ++curposy;
		}
	}
	return(update(buf,0));
}

int bkspc(void)
{
unsigned char buf[BUFSIZ],*p;
register int i;

	strcpy(buf,line->buffer);
	if(curposx) {
		i= -1;
		p= &line->buffer[curposx-1];
#ifdef KNJ
		if(curposx >= 2 && iskanji(line->buffer[curposx-2]) && iskanji2(line->buffer[curposx-1])) {
			--p;
			i= -2;
		}
#endif
		*p= '\0';
		sprintf(buf,"%s%s",line->buffer,&line->buffer[curposx]);
	}
	else {
		if(line->rev != NULL && ((i=strlen(line->rev->buffer))+strlen(line->buffer)) < (BUFSIZ-1)) {
			if(line->fwd == NULL) return(leftl());
			sprintf(buf,"\b%s%s",line->rev->buffer,line->buffer);
		}
		else return(NG);
	}
	return(update(buf,i));
}

int inson(void)
{
	insert ^= ON;
	inf();
	return(NG);
}

#if ((defined TEXT98) || (defined ATVRAM))
size_t scroll(int doff, int soff)
{
size_t cnt;
unsigned char __far *p;

	cnt = (lastdisplay - widbase - 2) * (displaylen << 1);
#ifdef TEXT98
	_fmemmove(&text_vram[doff], &text_vram[soff], cnt);
	_fmemmove(&attr_vram[doff], &attr_vram[soff], cnt);
#elif (defined ATVRAM)
	p = vram;
	_fmemmove(&p[(doff << 1)], &p[(soff << 1)], cnt);
#endif
	return(cnt);
}

int upl(void)
{
	int cnt;
	size_t bc;

	if(line->rev != NULL) {
		posflag= ON;
		line= line->rev;
		--curposy;
		if(curposy < widbase) {
			if(distop->rev != NULL) distop= distop->rev;
			++curposy;
			cnt = (widbase + 1) * displaylen;
			bc = scroll(cnt , widbase * displaylen);
			locate(1,1 + widbase); rcls(); putcrt(distop->buffer);
			vtflush((unsigned char *)vram + (cnt << 1), bc);
		}
		return(OK);
	}
	return(NG);
}

int dwnl(void)
{
	int cnt;
	size_t bc;

	if(line->fwd != NULL) {
		posflag= ON;
		line= line->fwd;
		if(curposy >= (lastdisplay - 2)) {
			if(distop->fwd != NULL) distop= distop->fwd;
			cnt = widbase * displaylen;
			bc = scroll(cnt , (widbase + 1) * displaylen);
			vtflush((unsigned char *)vram + (cnt << 1), bc);
			locate(1, lastdisplay - 1); rcls(); putcrt(line->buffer);
		}
		else curposy++;
		return(OK);
	}
	return(NG);
}
/*
#elif (defined UNIX)

int upl(void)
{
	register int i,cp;

	if(!ldup || widbase) {
		if(line->rev != NULL) {
			posflag= ON;
			line= line->rev;
			--curposy;
			if(curposy < widbase) {
				if(distop->rev != NULL) distop= distop->rev;
				++curposy;
				upscroll(0, widbase, distop->buffer);
			}
			return(OK);
		}
	}
	else {
		if(line->rev != NULL) {
			posflag= ON;
			line= line->rev;
			--curposy;
			cp= lastdisplay - widbase - 1;
			cp /= 2;
			if(curposy < widbase) {
				for(i= 0;i < cp;++i) {
					if(distop->rev != NULL) distop= distop->rev;
					else break;
					++curposy;
				}
				last= OFF;
			}
			return(OK);
		}
	}
	return(NG);
}

int dwnl(void)
{
	register int i,cp;

	if(!widbase) {
		if(line->fwd != NULL) {
			posflag= ON;
			line= line->fwd;
			if(curposy >= (lastdisplay - 2)) {
				if(distop->fwd != NULL) distop= distop->fwd;
				dwnscroll(0, lastdisplay - 2, line->buffer);
			}
			else curposy++;
			return(OK);
		}
	}
	else {
		if(line->fwd != NULL) {
			posflag= ON;
			line= line->fwd;
			curposy++;
			cp= lastdisplay - widbase - 1;
			cp /= 2;
			if(curposy >= (lastdisplay-1)) {
				for(i= 0;i < cp;++i) {
					if(distop->fwd != NULL) distop= distop->fwd;
					else break;
					--curposy;
				}
				last= OFF;
			}
			return(OK);
		}
	}
	return(NG);
}
*/
#else
int upl(void)
{
	register int i,cp;

	if(line->rev != NULL) {
		posflag= ON;
		line= line->rev;
		--curposy;
		cp= lastdisplay - widbase - 1;
		cp /= 2;
		if(curposy < widbase) {
			for(i= 0;i < cp;++i) {
				if(distop->rev != NULL) distop= distop->rev;
				else break;
				++curposy;
			}
			last= OFF;
		}
		return(OK);
	}
	return(NG);
}

int dwnl(void)
{
	register int i,cp;

	if(line->fwd != NULL) {
		posflag = ON;
		line = line->fwd;
		curposy++;
		cp = lastdisplay - widbase - 1;
		cp /= 2;
		if(curposy >= (lastdisplay-1)) {
			for(i = 0;i < cp;++i) {
				if(distop->fwd != NULL) distop= distop->fwd;
				else break;
				--curposy;
			}
			last= OFF;
		}
		return(OK);
	}
	return(NG);
}

#endif

chkword(int c)
{
	return((isalnum(c) || c == '_' ||c == '@' || c > 0x7f));
}

wordlwr(void)
{
	return(wrdchg(OFF));
}

worduper(void)
{
	return(wrdchg(ON));
}

wrdchg(int flg)
{
unsigned char *p,buf[BUFSIZ];
int chkword(int);
int n= 0;

	strcpy(buf,line->buffer);
	p= &buf[curposx];
	if(!chkword(*p)) {
		while(*p != '\0' && !chkword((int)*p)) {
			++p; ++n;
		}
	}
	if(*p == '\0') return(NG);
	while(*p != '\0' && chkword((int)*p)) {
		if(flg) *p = toupper(*p);
		else *p = tolower(*p);
		++p; ++n;
	}
	return(update(buf,n));
}

int rightl(void)
{
	extern int crp;

	if(line->buffer[curposx]) {
		posflag= ON;
		++curposx;
		return(OK);
	}
	if(dwnl() == NG) return(NG);
	crp = curposx= 0;
	return(OK);
}

int leftl(void)
{
	extern int crp;

	if(curposx) {
		posflag= ON;
		--curposx;
		return(OK);
	}
	if(upl() == NG) return(NG);
	curposx= strlen(line->buffer);
	crp = 0;
	return(OK);
}

fword(void)
{
	unsigned char *p;
	int chkword(int);

	p= line->buffer+curposx;
	if(!chkword(*p)) {
		while(*p != '\0' && !chkword((int)*p)) {
			++p; ++curposx;
		}
	}
	if(*p == '\0') {
		curposx= 0;
		return(dwnl());
	}
	while(*p != '\0' && chkword((int)*p)) {
		++p; ++curposx;
	}
	return(OK);
}

rword(void)
{
	unsigned char *p;
	int chkword(int);

	p= line->buffer+curposx;
	if(!curposx) return(leftl());
	do {
		--p; --curposx;
	} while(curposx && !chkword(*p));
	if(!curposx) return(NG);
	do {
		--p; --curposx;
	} while(curposx && chkword(*(p -1)));
	return(OK);
}

int pageup(void)
{
int i,s;

	s= NG;
	for(i=widbase;i < lastdisplay-1;++i) {
		if(distop->fwd != NULL) {
			distop= distop->fwd;
			s= ON;
		}
		else break;
	}
	if(s) {
		line= distop;
		curposx= 0;
		curposy= widbase;
		last= OFF;
		posflag= ON;
	}
	return(s);
}

int pagedwn(void)
{
	int i,s;

	s= NG;
	for(i= widbase;i < lastdisplay-1;++i) {
		if(distop->rev != NULL) {
			distop= distop->rev;
			s= ON;
		}
		else break;
	}
	if(s) {
		line= distop;
		curposx= 0;
		curposy= widbase;
		last= OFF;
		posflag= ON;
	}
	return(s);
}

int damy(void)
{
	msg("Hey don't touch !!\a\a\a");
	return(NG);
}

int macrost(void)
{
	unsigned short c;

	if(macroflg) {
		msg("Not now !\a");
		return(NG);
	}
	do {
		if((c =inputu("Key macro number = ",macronum,NULL)) == 0xffff) return(NG);
	} while(c >= MACROMAX);
	macronum= c;
	macropos= 0;
	macroflg= ON;
	return(NG);
}

int macroed(void)
{
	if(!macroflg) {
		msg("Not now !\a");
		return(NG);
	}
	macbuffer[macronum][macropos-1]= ENDMARK;
	macropos= 0;
	macroflg= OFF;
	msg("End of macro");
	return(NG);
}

int macrocd(void)
{
	unsigned short c;

	do {
		if((c =inputu("Key macro number = ",macronum,NULL)) == 0xffff) return(NG);
	} while(c >= MACROMAX);
	macronum= c;
	return(NG);
}

int macro(unsigned short c)
{
	int s;
	unsigned short *p;

        
	if(*macbuffer[macronum] == ENDMARK) return(NG);
	for(p= macbuffer[macronum];*p != ENDMARK;++p) {
		if(c == *p || execute(*p,1) == NG) return(NG);
	}
	return(OK);
}

leftgo(void)
{
	int s;

	s = (curposx)? OK:NG;
	curposx = 0;
	if(s) posflag= ON;
	return(s);
}

rightgo(void)
{
	register int i,s;

	s= NG;
	for(i= curposx;line->buffer[i] != '\0';++i) s= OK;
	curposx= i;
	if(s) posflag= ON;
	return(s);
}

cur_tab(void)
{
	register int i;

	for(i = (curposx - 1);i > 0;--i) if(line->buffer[i] != ' ' && line->buffer[i] != '\t') return OFF;
	return ON;
}

tabprc(void)
{
	register i;
	int cnt;
	unsigned char buf[BUFSIZ], *p;
	SCRN *lp;

	if(lng != C || !cur_tab()) return(charactor('\t'));
	cnt = k1count(backlp(line, curposx));
	if(coment) return(charactor('\t'));
	for(i = curposx;line->buffer[i] == '\t' || line->buffer[i] == ' ';++i) ;
	p = &line->buffer[i];
	if(p[0] == '/') {
		if(p[1] == '*') return(charactor('\t'));
		else if(p[1] == '/') return(charactor('\t'));
	}
	if(cnt && *p == '}') --cnt;
	for(i = 0;i < cnt;++i) buf[i] = '\t';
	buf[i] = '\0';
	strcat(buf, p);
	if(strcmp(buf, line->buffer)) {
		curposx = cnt;
		return(update(buf, 0));
	}
	return OK;
}

int tabkakko(void)
{
	register i;
	int cnt;
	unsigned char buf[BUFSIZ], ibuf[BUFSIZ], *p;

	cnt = k1count(backlp(line, curposx));
	if(coment) return(charactor('}'));
	if(cnt) --cnt;
	for(i = 0;i < cnt;++i) buf[i] = '\t';
	buf[i] = '\0';
	for(i = 0;i < curposx;++i) ibuf[i] = line->buffer[i];
	ibuf[i] = '\0';
	for(p = ibuf;*p == '\t' || *p == ' ';++p) ;
	strcat(buf, p);
	for(i = curposx;line->buffer[i] == '\t' || line->buffer[i] == ' ';++i) ;
	p = &line->buffer[i];
	strcat(buf, "}");
	strcat(buf, p);
	if(strcmp(buf, line->buffer)) {
		curposx = cnt + 1;
		return(update(buf, 0));
	}
	return OK;
}

int back_k_scan(int c)
{
	SCRN *lp, *slp = NULL;
	int ce, qt, kc, cmt, sx, i;
	unsigned char *p;
	extern posxflag;
	extern unsigned ungetdata;

	qt = cmt = 0;
	ce = (c == ']')? '[' : '(';
	if((lp = startk()) == NULL) return 0;
	p = lp->buffer;
	--curposx;
	kc = ksearch(ce, NULL);
	++curposx;
	if(coment) return NG;
	while(p != &line->buffer[curposx]) {
		if(!cmt && !qt && *p == c) ++kc;
		else if(!cmt && !qt && *p == ce) {
			--kc;
			if(kc == 0) {
				slp = lp;
				for(i = 0;p != &lp->buffer[i];++i) if(lp->buffer[i] == '\0') break;
				sx = i;
			}
		}
		else if(*p == '\0') {
			if((lp = nextlp(lp)) == NULL) break;
			p = lp->buffer;
			continue;
		}
		else if(*p == '\"') {
			if(qt == '\"') qt = 0;
			else if(qt == 0) qt = '\"';
			else ;
		}
		else if(*p == '\'') {
			if(p[1] == '\\' && p[3] == '\'') p = &p[3];
			else if(p[2] == '\'') p = &p[2];
			else if(qt == '\'') qt = 0;
			else if(qt == 0) qt = '\'';
			else ;
		}
		else if(*p == '/') {
			if(p[1] == '*') cmt = ON;
			else if(p[1] == '/') {
				if((lp = nextlp(lp)) == NULL) break;
				p = lp->buffer;
				continue;
			}
		}
		else if(cmt && *p == '*' && p[1] == '/') cmt = OFF;
		else if(*p == '\\') {
			if(p[1] == 'x' || p[1] == 'X') {
				do {
					++p;
				} while(isxdigit(*p)) ;
			}
			else if(isdigit(*p)) 
				while(isdigit(*p)) ++p;
			else p += 2;
			continue;
		}
		++p;
	}
	if(slp == NULL) return NG;
	if(slp != line) {
		cmt = 0;
		for(lp = distop;lp != slp;lp->fwd, ++cmt) if(lp == line || lp == NULL) return NG;
	}
	else {
		lp = line;
		cmt = curposy;
	}
	qt = curposx;
	curposx = sx;
	start_time();
	posxflag = OFF;
	curoff();
	locate(poschk(), cmt + 1); htprintf("%c", ce);
#ifdef UNIX
	ungetdata = getch();
#else
	while(!keysts()) {
		if(chk_time(3)) break;
	}
#endif
	curon();
	locate(poschk(), cmt + 1); ndprintf("%c", ce);
	curposx = qt;
	return OK;
}

int nonqt(void)
{
	int i, qt, cmt;
	unsigned char *p;

	if(!curposx) return 0;
	for(cmt = qt = i = 0;i < curposx;++i) {
		p = &line->buffer[i];
		if(*p == '\"') {
			if(qt == '\"') qt = 0;
			else if(qt == 0) qt = '\"';
			else ;
		}
		else if(*p == '\'') {
			if(p[1] == '\\' && p[3] == '\'') p = &p[3];
			else if(p[2] == '\'') p = &p[2];
			else if(qt == '\'') qt = 0;
			else if(qt == 0) qt = '\'';
			else ;
		}
		else if(*p == '/') {
			if(p[1] == '*') cmt = ON;
			else if(p[1] == '/') {
				cmt = ON;
				break;
			}
		}
		else if(cmt && *p == '*' && p[1] == '/') cmt = OFF;
		else ;
	}
	return(cmt + qt);
}

int kakko(unsigned short c)
{
	int s;

	if(lng == C) {
		if(c == '}' && !nonqt()) return(tabkakko());
		s = charactor(c);
		back_k_scan(c);
	}
	else s = charactor(c);
	return(s);
}

newline(void)
{
	return(addline(OFF));
}

indent(void)
{
	return(addline(ON));
}

addline(int flag)
{
	register int ps,i;
	unsigned char buf[BUFSIZ],ibuf[BUFSIZ],*p;

	ps= 0;
	if(flag) {
		if(lng == C) {
			i = k1count(backlp(line, curposx));
			while(i--) ibuf[ps++]= '\t';
		}
		else
		for(p= line->buffer;*p == ' ' || *p == '\t';++p) ibuf[ps++]= *p;
	}
	ibuf[ps]= '\0';
	for(i=0;i < curposx;++i) buf[i]= line->buffer[i];
	buf[i++]= '\n'; buf[i++]= '\0';
	strcat(buf,ibuf);
	strcat(buf,line->buffer+curposx);
	i= update(buf,0);
	curposx= ps;
	return(i);
}

int leftcut(void)
{
	unsigned char buf[BUFSIZ],ps[BUFSIZ],*p;
	int st;

	strcpy(buf,line->buffer);
	for(p= buf,st= 0;*p == ' ' || *p == '\t';++p,++st) ps[st]= *p;
	ps[st]= '\0';
	buf[curposx]= '\0';
	if((st= strlen(p)) == 0) return(NG);
	if(posflag) cutcpy(p);
	else cutcat(p);
	strcpy(buf,ps);
	strcat(buf,&line->buffer[curposx]);
	st= update(buf,-st);
	posflag= OFF;
	return(st);
}

int rightcut(void)
{
	unsigned char buf[BUFSIZ],*p;
	int st;

	strcpy(buf,line->buffer);
	p= &buf[curposx];
	if(*p == '\0') return(NG);
	if(posflag) cutcpy(p);
	else cutcat(p);
	*p= '\0';
	st= update(buf,0);
	posflag= OFF;
	return(st);
}

int linecut(void)
{
	unsigned char buf[BUFSIZ+1];
	int st;

	strcpy(buf,line->buffer);
	if(line->fwd != NULL) strcat(buf,"\n");
	if(posflag) cutcpy(buf);
	else cutcat(buf);
	st= NG;
	if(line->rev != NULL && strlen(line->rev->buffer) < (BUFSIZ-1)) {
		sprintf(buf,"\b%s",line->rev->buffer);
		st= update(buf,0);
		curposx= 0;
		if(line->fwd != NULL) {
			++curposy;
			line= line->fwd;
		}
	}
	else {
		editfile[edfile].curline= editfile[edfile].topline= distop= line->fwd;
		editfile[edfile].writeflag = ON;
		line->fwd->rev= NULL;
		strfree(line->buffer);
		linefree(line);
		last= OFF;
		line= distop;
		st= OK;
	}
	if(ldup && curposy > (lastdisplay/2)) last= refr_flg= OFF;
	posflag= OFF;
	return(st);
}

unsigned char *charbuf(unsigned char *p,int *st)
{
	unsigned char buf[BUFSIZ];
	register i;

	for(i=0;*p != '\0' && *p != '\n';++p,++i)
		buf[i]= *p;
	buf[i]= '\0';
	*st= update(buf,i);
	return(p);
}

int paste(void)
{
	unsigned char buf[BUFSIZ];
	unsigned char *m,*p,*charbuf(unsigned char *,int *);
	int s,x;
	SCRN *lp;

	x= curposx; lp= line;
	strcpy(buf,line->buffer); buf[x]= '\0';
	if((m= malloc((size_t)(strlen(cutbuf)+strlen(line->buffer)+1))) == NULL)
		return(NG);
	sprintf(m,"%s%s%s",buf,cutbuf,&line->buffer[x]);
	for(p= m;*p != '\0'; ) {
		if(*p == '\n') {
			s= newline();
			++p;
		}
		else p= charbuf(p,&s);
		if(s == NG) break;
	}
	line= lp;
	curposx= x;
	if((curposy= setcury(line)) == EOF) {
		refr_flg= 0; curposy= disset(5); last= OFF;
	}
	refr_flg= 0;
	return(s);
}

int logical(const unsigned char *buf,int px)
{
	register int x, i;

	for(i = 0, x = 0;i < px; ++i) {
		if(buf[i] == '\t') x += tabs - (x % tabs);
		else if(buf[i] == '\0') {
			cnv_err = i;
			return(x);
		}
		else if(buf[i] < ' ') x += 2;
		else ++x;
	}
	cnv_err = OFF;
	return(x);
}

int phygical(const unsigned char *buf,int px)
{
	register int x, i;

	for(i = 0, x = 0;px > x;++i) {
		if(buf[i] == '\t') x += tabs - (x % tabs);
		else if(buf[i] == '\0') {
			cnv_err = EOF;
			return(i);
		}
		else if(buf[i] < ' ') x += 2;
		else ++x;
	}
	cnv_err = (px == x)? OFF: ON;
	return(i);
}
