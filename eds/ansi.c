
#include <stdio.h>
#include <dos.h>
#include <conio.h>
#ifdef __TURBOC__
#include <alloc.h>
#include <mem.h>
#else
#include <malloc.h>
#include <graph.h>
#endif
#include <string.h>

#define REVCOLOR      "\x1b[7m"

#if (defined __TURBOC__)
#define        _ffree          free
#define     _fmalloc        malloc
#define   trsfer(d,s,b)   memcpy((d),(s),(b))

#elif (defined _MSC_VER)

#include <jstring.h>
#include <jctype.h>
#define       trsfer(d,s,b)   _fmemcpy((d),(s),(b))

#endif

extern int lastdisplay;
extern int ldup;

void vfputc(int c)
{
   putch(c);
}

void cnputs(char *p)
{
 cputs(p);
       fflush(stdout);
}

#ifdef ANSITERM

void vtputc(int c)
{
      vfputc(c);
}

void vtputs(char *p)
{
        while(*p) vfputc(*p++);
}

void cursav(void)
{
      cnputs("\x1b[s");
}

void curlod(void)
{
    cnputs("\x1b[u");
}

ndprintf(fmt, pr1, pr2)
char *fmt;
unsigned pr1;
unsigned pr2;
{
   unsigned char buf[BUFSIZ], *sp;
 int i, fl;
      unsigned k, p, b;

       fl = i = 0;
     while(*fmt) {
           if(*fmt == '%') {
                       ++fmt;
                  if(*fmt == 's') {
                               sp = (fl == 0)? (unsigned char *)pr1: (unsigned char *)pr2;
                             while(*sp) buf[i++] = *sp++;
                    }
                       else if(*fmt == 'd' || *fmt == 'u') {
                           p = (fl == 0)? pr1: pr2;
                                for(k = 10;k <= p;k *= 10) ;
                            do {
                                    k /= 10;
                                        b = p / k;
                                      b %= 10;
                                        buf[i++] = b + '0';
                             } while(k > 1) ;
                        }
                       ++fl;
           }
               else if(*fmt == '\r' || *fmt == '\n') ;
         else if(*fmt == '\a') putch(7);
         else buf[i++] = *fmt;
           ++fmt;
  }
       buf[i] = '\0';
  cnputs(buf);
}

void hanten(void)
{
 cnputs(REVCOLOR);
}

void normal(void)
{
    cnputs("\x1b[m");
}

htprintf(fmt, pr1, pr2)
char *fmt;
unsigned pr1;
unsigned pr2;
{
  unsigned char buf[BUFSIZ], *sp;
 int i, fl;
      unsigned k, p, b;

       fl = i = 0;
     while(*fmt) {
           if(*fmt == '%') {
                       ++fmt;
                  if(*fmt == 's') {
                               sp = (fl == 0)? (unsigned char *)pr1: (unsigned char *)pr2;
                             while(*sp) buf[i++] = *sp++;
                    }
                       else if(*fmt == 'd' || *fmt == 'u') {
                           p = (fl == 0)? pr1: pr2;
                                for(k = 10;k <= p;k *= 10) ;
                            do {
                                    k /= 10;
                                        b = p / k;
                                      b %= 10;
                                        buf[i++] = b + '0';
                             } while(k > 1) ;
                        }
                       ++fl;
           }
               else if(*fmt == '\r' || *fmt == '\n') ;
         else if(*fmt == '\a') putch(7);
         else buf[i++] = *fmt;
           ++fmt;
  }
       buf[i] = '\0';
  hanten();
       cnputs(buf);
}

void up(register int n)
{
   if(!n) n= 1;
    cprintf("\x1b[%dA",n);
}

void dwn(register int n)
{
        if(!n) n= 1;
    cprintf("\x1b[%dB",n);
}

void right(register int n)
{
      if(!n) n= 1;
    cprintf("\x1b[%dC",n);
}

void left(register short n)
{
     if(!n) n= 1;
    cprintf("\x1b[%dD",n);
}

void cls(void)
{
  cnputs("\x1b[2J");
}

void rcls(void)
{
     cnputs("\x1b[K");
}

void locate(short x,short y)
{
 cprintf("\x1b[%d;%dH",y ,x );
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


# ifdef __386__

msg(fmt, pr1, pr2)
char *fmt;
unsigned pr1;
unsigned pr2;
{
        unsigned char *sp;
      int i, fl, y;
   unsigned k, p, b;
       char buf[BUFSIZ];

       y = (ldup == 0)? lastdisplay: ldup;
     fl = i = 0;
     while(*fmt) {
           if(*fmt == '%') {
                       ++fmt;
                  if(*fmt == 's') {
                               sp = (fl == 0)? (unsigned char *)pr1: (unsigned char *)pr2;
                             while(*sp) buf[i++] = *sp++;
                    }
                       else if(*fmt == 'd' || *fmt == 'u') {
                           p = (fl == 0)? pr1: pr2;
                                for(k = 10;k <= p;k *= 10) ;
                            do {
                                    k /= 10;
                                        b = p / k;
                                      b %= 10;
                                        buf[i++] = b + '0';
                             } while(k > 1) ;
                        }
                       ++fl;
           }
               else if(*fmt == '\r' || *fmt == '\n') ;
         else if(*fmt == '\a') putch(7);
         else buf[i++] = *fmt;
           ++fmt;
  }
       buf[i] = '\0';
  locate(1, y + 1);
       rcls();
 vtputs(buf);
}

# endif

#endif

char *crcut(char *p)
{
 while(*p != '\0')
               if(*p == '\n' || *p == '\r') *p= '\0';
          else ++p;
       return(p);
}


#ifdef TEXT98

#define ON                              1
#define OFF                            0

#ifdef __TURBOC__
#define       _far    far
#define      __far   far
#endif

#define TEXTSTART              ((void _far *)0xa0000000)
#define ATTRSTART              ((void _far *)0xa2000000)

#define TEXTEND                        ((void _far *)0xa0000fa0)
#define ATTREND                        ((void _far *)0xa2000fa0)
#define        VRAMSIZ                 ((size_t)0xfa0)

#define DEF_CUR          0xe1

#if (_MSC_VER >= 6)
#define _ei();                   _asm STI
#define _di();                  _asm CLI
#endif
#ifdef __TURBOC__
#define _ei()                    enable()
#define _di()                   disable()
#endif

#define  VRAMSEGMENT             0xa000

typedef struct {
  unsigned char tram;
     unsigned char gst;
} TEXT;

TEXT _far *text= TEXTSTART;
TEXT _far *attr= ATTRSTART;

TEXT _far *stext= TEXTSTART;
TEXT _far *sattr= ATTRSTART;

unsigned char rev_col = REV_COL;
unsigned char nor_col = DEF_CUR;
unsigned char defattr = DEF_CUR;
void _far *scrn[2];

int batch(char *,char *);

unsigned beut(register c)
{
static const char *cpright= 
       "Copyright (c) 1988 - 93 Nobby N Hirano All right reserved." ;

unsigned register po;

     if(c > ' ' &&  c < 0x60)
                po= c + 0x851f;
 else if(c > 0x60 && c < 0x80)
           po= c + 0x8520;
 else if(c > 0xa0 && c < 0xe0)
           po= c + 0x84fe;
 else return(0);
 po= jmstojis(po) - 0x2000;
      return(po);
}

void movecur(void)
{
#if (_MSC_VER >= 6)
      _asm {
          mov     ah,0x13
         mov     dx,WORD PTR attr
                int     0x18
    }
#elif __TURBOC__
union REGS ir,or;

      ir.h.ah = 0x13;
 ir.x.dx = (unsigned)((unsigned long)attr);
      int86(0x18, &ir, &or);
#endif
}

void cursav(void)
{
 stext = text;
   sattr = attr;
}

void curlod(void)
{
        text = stext;
   attr = sattr;
   movecur();
}

void right(register int n)
{
TEXT _far *tp;

    if(!n) n= 1;
    tp= &text[n];
   if(tp < TEXTEND) {
              text= tp;
               attr= &attr[n];
         movecur();
      }
}

void settram(unsigned c)
{
#if (_MSC_VER >= 6)
  _asm {
          push    es
              push    bx
              mov     ax,VRAMSEGMENT
          mov     es,ax
           mov     ax,c
            mov     bx,word ptr text
                mov     es:[bx],ah
              mov     es:[bx + 1],al
          add     bx,0x2000                       ;Adjust Attrib.
         mov     al,defattr
              mov     es:[bx],al
              add     word ptr attr,2
         add     word ptr text,2
         pop     bx
              pop     es
      }
#else

  text->tram = c >> 8;
    text->gst = c & 255;
    attr->tram = defattr & 0xff;
    ++attr; ++text;
#endif
}

#ifndef _MSC_VER
# ifndef __TURBOC__
void trsfer(void _far *di,void _far *sr,size_t bc);
{
register unsigned i;
unsigned char _far *da, *sa;

   da = di; sa = sr;
       for(i= 0;i < bc;++i) {
          _di();
          *da= *sa;
               _ei();
          ++da; ++sa;
     }
}
# endif
#endif

void curoff(void)
{
#if (_MSC_VER >= 6)
    _asm {
          mov     ah,0x12
         int     0x18
    }
#else
union REGS ir,or;

 ir.h.ah = 0x12;
 int86(0x18, &ir, &or);
  return(or.h.al);
#endif
}

void curon(void)
{
#if (_MSC_VER >= 6)
     _asm {
          mov     ah,0x11
         int     0x18
    }
#else
union REGS ir,or;

 ir.h.ah = 0x11;
 int86(0x18, &ir, &or);
  return(or.h.al);
#endif
}

void vtputc(int c)
{
static unsigned kflg= OFF;
char *p;
unsigned cd;
extern beut_flg;

        if(c == 7) putch('\a');
 else if(kflg) {
         kflg <<= 8;
             kflg += c;
              cd= jmstojis(kflg) - 0x2000;
            settram(cd);
            cd |= 0x8000;
           settram(cd);
            kflg= OFF;
      }
       else if(iskanji(c)) {
           kflg= c;
                return;
 }
       else if(beut_flg && (cd= beut(c)) != 0)
         settram(cd);
    else 
           settram((unsigned)c << 8);
}


void vtputs(char *p)
{
        while(*p) vtputc(*p++);
}

int ndprintf(fmt)
char *fmt;
{
va_list argptr;
char buf[BUFSIZ];
int l;

       va_start(argptr,fmt);
   l= vsprintf(buf,fmt,argptr);
    va_end(argptr);
 defattr= nor_col;
       vtputs(buf);
    return(l);
}

int htprintf(fmt)
char *fmt;
{
va_list argptr;
char buf[BUFSIZ];
int l;

   va_start(argptr,fmt);
   l= vsprintf(buf,fmt,argptr);
    va_end(argptr);
 defattr= rev_col;
       vtputs(buf);
    defattr= nor_col;
       return(l);
}

void up(register int n)
{
TEXT _far *tp;

       if(!n) n= 1;
    tp= text; tp -= 80*n;
   if(tp >= TEXTSTART) {
           text= tp;
               attr -= 80*n;
           movecur();
      }
}

void dwn(register int n)
{
TEXT _far *tp;

       if(!n) n= 1;
    tp= &text[80 * n];
      if(tp < TEXTEND) {
              text= tp;
               attr= &attr[80 * n];
            movecur();
      }
}

void left(register short n)
{
TEXT _far *tp;

    if(!n) n= 1;
    tp = text - n;
  if(tp >= TEXTSTART) {
           text= tp;
               attr -= n;
              movecur();
      }
}

void cls(void)
{
TEXT _far *tp;
register i;

      _di();
  for(i= 0,tp= TEXTSTART;i < 2000;++tp,++i) {
             tp->tram= ' '; tp->gst= 0;
      }
       for(i= 0,tp= ATTRSTART;i < 2000;++tp,++i) tp->tram= nor_col;
    _ei();
  text= TEXTSTART;
        attr= ATTRSTART;
        movecur();
}

void rcls(void)
{
TEXT _far *tp,*ap;

   ap= attr; tp= text;
     _di();
  do {
            tp->tram= ' '; tp->gst= 0;
              ap->tram= nor_col;
              ++tp; ++ap;
     } while((FP_OFF(tp) % 160) != 0) ;
      _ei();
}

void locate(short x,short y)
{
TEXT _far *tp= TEXTSTART;
TEXT _far *ap= ATTRSTART;
static short xx = 0, yy = 0;

      if(xx == x && yy == y) return;
  --x, --y;
       x += y * 80;
    text= &tp[x];
   attr= &ap[x];
   movecur();
      xx = x + 1; yy = y + 1;
}

void pushscrn(void)
{
int i;

      cnputs("\x1b[s");
       for(i= 0;i < 2;++i) {
           scrn[i]= _fmalloc(VRAMSIZ);
             if(!chkmemerr(scrn[i])) return;
 }
       trsfer(scrn[0],TEXTSTART,VRAMSIZ);
      trsfer(scrn[1],ATTRSTART,VRAMSIZ);
      locate(1, 25); rcls();
  cnputs("\x1b[1h");
      batch(NULL,"///");
      initfkey();
}

void popscrn(void)
{
 batch("///",NULL);
      backfkey();
     cnputs("\x1b[1l");
      trsfer(TEXTSTART,scrn[0],VRAMSIZ);
      trsfer(ATTRSTART,scrn[1],VRAMSIZ);
      _ffree(scrn[0]); _ffree(scrn[1]);
       cnputs("\x1b[u");
}

#endif

void sppcnt(int n,int htflg)
{
register i;

#ifdef ANSITERM
 if(htflg) hanten();
#endif
#ifdef TEXT98
  if(htflg) defattr= rev_col;
#endif

       for(i =0;i < n;++i) vtputc(' ');

#ifdef ANSITERM
 if(htflg) normal();
#endif
#ifdef TEXT98
  defattr= nor_col;
#endif
}

#ifdef AT

void vtputs(char *p)
{
  cnputs(p);
}

void vtputc(int c)
{
#if (_MSC_VER >= 6)
       _asm {
          mov     ah,2
            mov     dl,c
            int     21h
     }
#else
union REGS ir,or

  ir.h.ah = 2;
    intdos(&ir, &or);
#endif
}

void cursav(void)
{
      cnputs("\x1b[s");
}

void curlod(void)
{
    cnputs("\x1b[u");
}

void ndprintf(fmt)
char *fmt;
{
va_list argptr;
char buf[BUFSIZ];

  va_start(argptr,fmt);
   vsprintf(buf,fmt,argptr);
       va_end(argptr);
 cnputs(buf);
}

void hanten(void)
{
 cnputs(REVCOLOR);
}

void normal(void)
{
    cnputs("\x1b[m");
}

void htprintf(fmt)
char *fmt;
{
va_list argptr;
char buf[BUFSIZ];

 va_start(argptr,fmt);
   vsprintf(buf,fmt,argptr);
       va_end(argptr);
 hanten();
       cnputs(buf);
    normal();
}

void up(register int n)
{
      if(!n) n= 1;
    ndprintf("\x1b[%dA",n);
}

void dwn(register int n)
{
        if(!n) n= 1;
    ndprintf("\x1b[%dB",n);
}

void right(register int n)
{
      if(!n) n= 1;
    ndprintf("\x1b[%dC",n);
}

void left(register short n)
{
     if(!n) n= 1;
    ndprintf("\x1b[%dD",n);
}

void cls(void)
{
  cnputs("\x1b[2J");
}

void rcls(void)
{
     cnputs("\x1b[K");
}

void locate(short x,short y)
{
 ndprintf("\x1b[%d;%dH",y ,x );
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
#endif
