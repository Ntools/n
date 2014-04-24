#include "ed.h"
#ifdef _MSC_VER
# include <search.h>
#endif
#ifdef TEXT98
#  include <98key.h>
#endif
#ifdef UNIX
unsigned short eofchar= 7;
#else
# ifdef KNJ
#     include <jctype.h>
# endif
#endif

#define    END     -1
#define CANCEL        "Cancel !!\a"

int crp= 0;
int lup= OFF;
int upflag= OFF;
int posxflag = OFF;
extern cnv_err;
extern unsigned short eofchar;

/* Command */

extern struct KEY key[];
extern lng;

keycount()
{
	for(key_count = 0;key[key_count].func != (void *)0;++key_count) ;
	keyptr = key;
    return(key_count);
}

void edmain(unsigned char *nam, int flag)
{
int s;
int editexec(void);
extern emsflag,emssize;

	cls();
	if(fileset(nam, flag) == NG) return;
#ifndef UNIX
	if(emsflag) msg("EMM active geting %dPage(s) %dKbytes.",emssize,(emssize*16));
#endif
	do {
		s = editexec();
		posxflag = OFF;
	} while(s != END);
#ifdef EMS98
	if(emsflag) emsfree();
#endif
}

unsigned inputu(unsigned char *fmt, int cnt, unsigned short *keycode)
{
unsigned char buf[7];
int i,j;
unsigned short c;

	if(keycode) *keycode= 0;
	msg("%s%d",fmt,cnt);
	i = (ldup == 0)? lastdisplay+1: ldup+1;
	locate(1,i);
	right(strlen(fmt));
	buf[0]= '\0'; i= 0;
	do {
		if((c= getkey()) == '\r') break;
		else if(isdigit(c)) {
			buf[i++]= c;
			buf[i]= '\0';
			vtputc(c);
		}
		else if(c == eofchar) {
			msg(CANCEL);
			return(0xffff);
		}
		else if(c == TIMES) {
			buf[i]= '\0';
			rcls();
		}
#ifdef UNIX
		else if(c == BSPC || c == 0x110)
#else
		else if(c == BSPC)
#endif
		{
			if(i) {
				for(j= i;buf[j] != '\0';++j) buf[j-1]= buf[j];
				buf[j-1]= '\0';
				i--; left(1); rcls();
				if(buf[i]) {
					ndprintf("%s",&buf[i]);
					left(strlen(&buf[i]));
				}
			}
		}
		else {
			if(keycode) {
				*keycode= c;
				break;
			}
		}
#ifdef TEXT98
		movecur();
#endif
	} while(i < 7);
	rcls();
	if(buf[0]) {
		c = 0;
		for(i = 0;buf[i] != '\0';++i) {
			c *= 10;
			c += buf[i] - '0';
		}
	}
	else c = cnt;
	return(c);
}

int charkey(unsigned short k) /* if charactor then ON */
{
	if(k & 0xff00) return(OFF);
	else if(k == '}') return(K11_TYPE);             /* Version 5.0 Additional */
    else if(k == ')') return(K12_TYPE);             /* Version 5.0 Additional */
    else if(k == ']') return(K13_TYPE);             /* Version 5.0 Additional */
    else if(k == 0x7f || k == 0xff || k < ' ') return(OFF);
	return(ON);
}

cpkey(void const *kp1,void const *kp2)
{
     return(stricmp(((struct KEY *)kp1)->keyword,((struct KEY *)kp2)->keyword));
}

codekey(void const *kp1,void const *kp2)
{
   return(((struct KEY *)kp1)->code - ((struct KEY *)kp2)->code);
}

int execute(unsigned short k,unsigned cnt)
{
    struct KEY *kp,kk;
    int s;

    if(last && !(macroflg && macropos < MACROBUF-1) && upflag) msg("");
    while(cnt--) {
		if((s = charkey(k)) == ON) s = charactor(k);
		else if(s >= K11_TYPE && s <= K13_TYPE) {
			s = kakko(k);
		}
		else if(k == '\t') s = tabprc();
		else {
			kk.code = k;
			kp = lfind(&kk, keyptr, &key_count,sizeof(struct KEY),codekey);
			if(kp == NULL) {
#ifdef UNIX
				if(kpadsearch(k, &s) == NG) { /* msg("No assign Key !\a"); */
//					msg("Key=0x%06x", k);
				}
#else
				msg("No assign Key !\a");
				s = NG;
#endif
			}
			else s = (*kp->func)(k);
		}
		if(s == NG) break;
	}
	if(s == 0xff) s = OK;
	return(s);
}

int editexec(void)
{
	unsigned short keycode = 0;
	unsigned c,c2;
	static unsigned int cnt= 1;
	unsigned t= 1;
	int st,fn;
	void lineinf(void);

	if(lup) linenum();
	lup= OFF;
	st= poschk();
	if(!last) display();
	lineinf();
	locate(st,curposy+1);
	c = getkey();
	if(c == TIMES) {
		if((t = inputu("How many = ", cnt, &keycode)) != 0xffff) cnt= t;
		else return(OK);
	}
	else if(!(keycode & 0xff00)) {
		if(c == METACHAR || c == EXTEND) {
			if(c == EXTEND) {
				keycode= CTRX;
			}
			else {
				keycode= META;
			}
			c2 = getkey();
			if(c != CTRL('@'))
				c2 = toupper((int)c2);
			keycode |= c2;
		}
		else keycode = c;
	}
	if(t > 1) last = OFF;
	else last = ON;
	fn = edfile;
	if(macroflg && macropos < MACROBUF-1)
		macbuffer[macronum][macropos++] = keycode;
	if(curposx > (c = strlen(line->buffer))) curposx = c;
	st = execute(keycode,t);
	if(edfile != fn) tablenset();
	t = 1;
	keycode = 0;
	editfile[edfile].curline = line;
	return(st);
}

doscom(void)
{
	static unsigned char cmdbuf[81]= "grep ";
	unsigned char buf[BUFSIZ];
	int s;
	extern refr_flg;

	strcpy(buf,cmdbuf);
	if(inputs(buf,"Operating system command: ") == NULL) return(NG);
	if(buf[0]) strcpy(cmdbuf,buf);
	else return(NG);
	curon();
#ifdef TEXT98
	popscrn();
	printf("%s\n",cmdbuf);
#endif
	s= system(cmdbuf);
	cnputs("\r\nHit any Key !"); getkey();
#ifdef TEXT98
	pushscrn();
#endif
	locate(1, (ldup == 0)? lastdisplay+1: ldup+1); rcls();
	curon();
	redisplay(CTRL('G'));
	return(s);
}

shell(void)
{
 int s;
  extern refr_flg,ldup;
    unsigned char *sh;

#ifdef MSDOS
  if((sh= getenv("COMSPEC")) == NULL) sh= "Command";
#endif

#if (defined TEXT98)
    popscrn();
#elif (defined UNIX)
  ttclose();
      if((sh= getenv("SHELL")) == NULL) sh= "csh";
#else
       batch("///",NULL);
#endif
        cnputs("if end to shell type 'exit' !\r\n");
    s= system(sh);
#if (defined TEXT98)
      pushscrn();
     locate(1, (ldup == 0)? lastdisplay+1: ldup+1); rcls();
#elif (defined UNIX)
      ttopen();
       if((sh= getenv("SHELL")) == NULL) sh= "csh";
#else
       batch(NULL,"///");
#endif
        redisplay(CTRL('G'));
   return(s);
}

charin(unsigned char buf[],unsigned char *ms)
{
	int i;
	register j;
	unsigned short c;

#ifdef UNIX
	i= strlen(buf);
#else
	i= strlen(buf) - 1;
#endif

	msg("%s%s",ms,buf);
#ifdef TEXT98
	movecur();
#endif
	while(i > 0) {
		--i;
		left(1);
	}
	i= c= 0;
	do {
		c |= getkey();
		if(c == eofchar) {
			msg(CANCEL);
			return(EOF);
		}
		else if(c == METACHAR) {
			c= META;
			continue;
		}
		else if(c == RIGHTCUT) {
			buf[i]= '\0';
			rcls();
		}
#ifdef UNIX
		else if(c == INSON || c == 0x112)
#else
		else if(c == INSON)
#endif
		{
			insert ^= ON;
			cursav();
			inf();
			curlod();
		}
#ifdef UNIX
		else if(c == DELCHAR || c == 0x113) 
#else
		else if(c == DELCHAR) 
#endif
		{
			if(buf[i] != '\0') {
				for(j= i;buf[j] != '\0';++j) buf[j]= buf[j+1];
				rcls();
				if(buf[i]) {
					ndprintf("%s",&buf[i]);
					left(strlen(&buf[i]));
				}
			}
		}
		else if(c == BEGLINE) {
			while(i) {
				--i; left(1);
			}
		}
		else if(c == ENDLINE) {
			while(buf[i] != '\0') {
				++i; right(1);
			}
		}
#ifdef UNIX
		else if(c == BSPC || c == 0x110)
#else
		else if(c == BSPC)
#endif
		{
			if(i) {
				for(j= i;buf[j] != '\0';++j) buf[j-1]= buf[j];
				buf[j-1]= '\0';
				i--; left(1); rcls();
				if(buf[i]) {
					ndprintf("%s",&buf[i]);
					left(strlen(&buf[i]));
				}
			}
		}
#ifdef UNIX
		else if(c == RIGHTCHAR || c == 0x104)
#else
		else if(c == RIGHTCHAR)
#endif
		{
			if(buf[i] != '\0') {
				++i; right(1);
			}
		}
#ifdef UNIX
		else if(c == LEFTCHAR || c == 0x103)
#else
		else if(c == LEFTCHAR)
#endif
		{
			if(i) {
				--i; left(1);
			}
		}
		else if(c == '\r') break;
		else if(!(c & 0xff00)) {
			vtputc(c);
			if(insert) {
				for(j= i;buf[j] != '\0';++j) ;
				while(j >= i) {
					buf[j+1]= buf[j];
					--j;
				}
				if(buf[i] != '\0') {
					ndprintf("%s",&buf[i+1]);
					left(strlen(&buf[i+1]));
				}
			}
			else {
				if(buf[i] == '\0') buf[i+1]= '\0';
			}
			buf[i++]= c;
		}
		c = 0;
#ifdef TEXT98
		movecur();
#endif
	} while(i < 80);
	return(c);
}

unsigned char *inputs(unsigned char *buf,unsigned char *m)
{
	int s;

	do {
		s= charin(buf,m);
		if(s == EOF) return(NULL);
	} while(s != '\r');
	return(buf);
}

int poschk(void)
{
	register int x,i;
	unsigned char *p;
	int st;
	extern refr_flg;
	static int xx = 0, yy = 0;
	static int lx = 0, px = 0;

	if(posxflag) return(px);
	posxflag = ON;
	x = logical(line->buffer, curposx);
	if(curposy != yy) {
		if(cnv_err != OFF) {
			if(crp < lx) crp = lx;
			curposx = cnv_err;
		}
		else if(crp) {
			curposx = phygical(line->buffer, crp);
			x = logical(line->buffer, curposx);
		}
	}
#ifdef KNJ
	i = curposx;
	if(i--) {
		if(iskanji(line->buffer[i++]) && iskanji2(line->buffer[i])) {
			if(curposx > xx) {
				++curposx;
				++x;
			}
			else if(curposx < xx) {
				--curposx;
				--x;
			}
		}
	}
#endif
	if(curposy == yy) crp = x;
	lx = x;
	x++;
	if(rightshift && (x-rightshift) < displaylen && (x-rightshift) > (displaylen >> 1)) {
		st = rightshift;
		x -= st;
	}
       else if(x >= displaylen) {
		st = (x / 10) * 10 - 70;
		x -= st;
        }
       else st = 0;
    if(st != rightshift) refr_flg = last = OFF;
      rightshift = st;
        xx = curposx;
   yy = curposy;
   px = x;
 return(x);
}

#ifdef MSDOS
unsigned short KEYB()
{
#if (_MSC_VER >= 6)
        _asm {
          mov     ah,7
            int     0x21
            xor     ah,ah
   }
#else
union REGS ir,or;

 ir.h.ah = 7;
    intdos(&ir, &or);
       return(or.h.al);
#endif
}
#endif

compil()
{
unsigned char buf1[80];
unsigned char buf2[BUFSIZ];
extern unsigned char *tpnam;

    if(lng == EOF) {
                msg("I don't know how to compile.\a");
          return(NG);
     }
       sprintf(buf1,copl[lng],editfile[edfile].filename);
      sprintf(buf2,"errout -f nul %s >%s",buf1,tpnam);
        if(editfile[edfile].writeflag) {
                if(ynchk("File save") == YES)
                   save(editfile[edfile].topline,editfile[edfile].filename);
               else return(NG);
        }
       msg("%s",buf1);
 system(buf2);
   errorfile();
    return(NG);
}
