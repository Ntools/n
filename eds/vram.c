#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <jctype.h>
#include <dos.h>

#if (defined _MSC_VER)
# include <memory.h>
# define        trsfer(d,s,b)   _fmemcpy((d),(s),(b))
#endif

#if (defined __386__)
# define        trsfer(d,s,b)   _fmemcpy((d),(s),(b))
# define   BellCall()              vfputc(7)
#endif

#define          REV_ATTR        0x70
#define             DEF_ATTR        0x1f
#define             CLSCHAR         ((DEF_ATTR << 8)+' ')

#define            JDOS            0                       /* PS/55 擔杮岅DOS */
#define            PCDOS           1                       /* PC DOS */
#define             DOSVJ           2                       /* DOS/V 擔杮岅影霓 */
#define           DOSVE           3                       /* DOS/V 塸岅影霓 */
#define             AXJ                     4                       /* AX 擔杮岅影霓 */
#define              AXE                     5                       /* AX 塸岅影霓 */
#define                J3100J          6                       /* J3100 擔杮岅影霓 */
#define           J3100E          7                       /* J3100 塸岅影霓 */
#define             DRDOSJ          8                       /* DR-DOS 擔杮岅影霓 */
#define          DRDOSE          9                       /* DR-DOS 塸岅影霓 */
#define            PC9801          10                      /* PC-9801 */
#define            PC9801H         11                      /* PC-9801 Hireso */

void __far *vram = NULL;
void __far *curp = NULL;
void *savp = NULL;
int initx = 0;
int inity = 0;
int posx = 0;
int posy = 0;
int sposx = 0;
int sposy = 0;
unsigned short cursor = 0;
unsigned short attr = DEF_ATTR;

extern int displaylen;
extern int lastdisplay;
extern int ldup;

struct    SREGS sr;

#ifndef __386__

union   REGS ir,or;

#else

typedef struct {
        unsigned long EDI;
      unsigned long ESI;
      unsigned long EBP;
      unsigned long RSV;
      unsigned long EBX;
      unsigned long EDX;
      unsigned long ECX;
      unsigned long EAX;
      unsigned short FLAGS;
   unsigned short ES;
      unsigned short DS;
      unsigned short FS;
      unsigned short GS;
      unsigned short IP;
      unsigned short CS;
      unsigned short SP;
      unsigned short SS;
} REAL_REGS;
REAL_REGS ir,or;

unsigned short real_seg;

#endif

unsigned char *machinename[] = {
    "PS/55 擔杮岅DOS",
      "PC DOS",
       "DOS/V 擔杮岅影霓",
     "DOS/V English mode",
   "AX 擔杮岅影霓",
        "AX English mode",
      "J3100 擔杮岅影霓",
     "J3100 English mode",
   "DR-DOS 擔杮岅影霓",
    "DR-DOS English mode",
  "PC-9801",
      "PC-9801 Hireso",
       NULL
};

#ifdef __386__
intr86(unsigned short intnum, REAL_REGS __far *inregs, REAL_REGS __far *outregs)
{
   union   REGS ireg,oreg;
 struct  SREGS sreg;
     REAL_REGS __far *rr;

    segread(&sreg);
 ireg.w.ax  = 0x300;
     ireg.w.cx  = 0;
 ireg.h.bl  = intnum & 255;
      ireg.h.bh  = 0;
 sreg.es    = FP_SEG(inregs);
    ireg.x.edi = FP_OFF(inregs);
    int386x(0x31, &ireg, &oreg, &sreg);
     rr = MK_FP(sreg.es, oreg.x.edi);
        *outregs = *rr;
 return(oreg.x.cflag);
}
#endif

void vfputc(int c)
{
#ifdef __386__
   ir.EAX = c;
     intr86(0x29,&ir, &or);
#else
     ir.x.ax = c;
    int86(0x29,&ir, &or);
#endif
}

void cnputs(unsigned char *p)
{
      while(*p) vfputc(*p++);
}

unsigned char *crcut(unsigned char *p)
{
 while(*p != '\0')
               if(*p == '\n' || *p == '\r') *p= '\0';
          else ++p;
       return(p);
}

void getcpos(int __far *xptr, int __far *yptr)
{
#ifdef __386__
        ir.EAX = 0x300;
 ir.EBX = 0;
     intr86(0x10, &ir, &or);
 *xptr = (or.EDX & 0xff) + 1;                    /* dl + 1 */
    *yptr = ((or.EDX & 0xff00) >> 8) + 1;   /* dh + 1 */
#else
       __asm {
         mov     ah, 3
           xor     bh, bh
          int     0x10
            add     dx, 0x0101
              xor     ah, ah
          les     bx, xptr
                mov     al, dl
          mov     word ptr es:[bx], ax
            les     bx, yptr
                mov     al, dh
          mov     word ptr es:[bx], ax
    }
#endif
}

void __far *vramptr(void)
{
#ifdef __386__
        union   REGS ireg,oreg;

 ir.ES = 0xb800;
 ir.EDI = 0;
     ir.EAX = 0xfe00;
        intr86(0x10, &ir, &or);
 real_seg = ireg.w.bx = or.ES;
   ireg.w.ax = 2;
  int386(0x31, &ireg, &oreg);
     if(oreg.x.cflag != 0) return(NULL);
     return((void __far *)MK_FP(oreg.w.ax, or.EDI));
#else
    __asm {
         push    di
              mov     ax, 0xb800
              mov     es, ax
          xor     di, di
          mov     ah, 0xfe
                int     0x10
            mov     dx, es
          mov     ax, di
          pop     di
      }
#endif
}

void vtflush(void __far *vs, size_t sz)
{
#ifdef __386__
  ir.ES = real_seg;
       ir.EDI = FP_OFF(vs);
    ir.ECX = sz;
    ir.EAX = 0xff00;
        intr86(0x10, &ir, &or);
#else
    __asm {
         push    di
              les     di, vs
          mov     cx, sz
          mov     ah, 0xff
                int     0x10
    }
#endif
}

void __far *curptr(void)
{
unsigned short tsz;

    tsz = FP_OFF(curp) - FP_OFF(vram);
      tsz >>= 1;
      posy = tsz / displaylen;
        posx = tsz % displaylen;
        return(curp);
}

void movecur(void)
{
#ifdef __386__
 curptr();
       ir.EBX = 0;
     ir.EDX = ((posy & 0xff) << 8) + (posx & 0xff);
  ir.EAX = 0x200;
 intr86(0x10, &ir, &or);
#else
    curptr();
       __asm {
         xor     bh, bh
          mov     dh, byte ptr posy
               mov     dl, byte ptr posx
               mov     ah, 2
           int 0x10
        }
#endif
}

void locate(int x,int y)
{
unsigned short __far *tp;

      --x; --y;
       if(posx == x && posy == y) return;
      tp = vram;
      curp = &tp[((displaylen * y) + x)];
     movecur();
}

void cls(void)
{
unsigned short __far *tp;
register i, j;
unsigned x, y;

#ifdef __386__
   x = *((unsigned short __far *)MK_FP(0x40, 0x4a));
       y = *((unsigned char __far *) MK_FP(0x40, 0x84)) + 1;
#else
      x = *((unsigned short __far *)0x0040004a);
      y = *((unsigned char __far *)0x00400084) + 1;
#endif
     tp = vram;
      for(i= 0;i < y;++i) {
           for(j = 0;j < x;++tp, ++j) *tp = CLSCHAR;
       }
       vtflush(vram, x * y);
   locate(1, 1);
}

void rcls(void)
{
unsigned short __far *tp;
register i, j;

   tp = curp;
      for(j = 0, i = posx;i < displaylen;++tp, ++i, ++j) *tp = CLSCHAR;
       vtflush(curp, j);
}

void vbputc(unsigned c)
{
unsigned char __far *tp;

      if(iskanji(c))
          tp = curp;
      if(c == '\a') putch('\a');
      else {
          tp = curp;
              *tp++ = c; *tp++ = attr;
                curp = tp;
      }
}

vtputs(unsigned char *buf)
{
register i;
unsigned short __far *tp;

       for(tp = curp, i = 0;*buf != '\0';++i, ++buf) vbputc(*buf);
     vtflush(tp, i);
 movecur();
}

vtputc(unsigned c)
{
  vbputc(c);
      vtflush((unsigned char __far *)curp - 2, 1);
    movecur();
}

void hanten(void)
{
   attr = REV_ATTR;
}

void normal(void)
{
     attr = DEF_ATTR;
}


#if (defined __386__)

ndprintf(unsigned char * format, ...)
{
     va_list arglist;
        unsigned char buf[BUFSIZ];

    va_start( arglist, format );
    vsprintf( buf, format, arglist );
    va_end( arglist );

   attr= DEF_ATTR;
 vtputs(buf);
}

htprintf(unsigned char * format, ...)
{
     va_list arglist;
        unsigned char buf[BUFSIZ];

    va_start( arglist, format );
    vsprintf( buf, format, arglist );
    va_end( arglist );

   attr= REV_ATTR;
 vtputs(buf);
    attr= DEF_ATTR;
}

msg(unsigned char * format, ...)
{
       va_list arglist;
        unsigned char buf[BUFSIZ];
      int x, y;

    va_start( arglist, format );
    vsprintf( buf, format, arglist );
    va_end( arglist );

    y = (ldup == 0)? lastdisplay+1: ldup+1;
 locate(1, y);
   rcls();
 buf[79]= '\0';
  x = strlen(buf);
        attr = DEF_ATTR;
        vtputs(buf);
    locate(x, y);
}

#else
int ndprintf(fmt)
unsigned char *fmt;
{
va_list argptr;
unsigned char buf[BUFSIZ];
int l;

  va_start(argptr,fmt);
   l= vsprintf(buf,fmt,argptr);
    va_end(argptr);
 attr= DEF_ATTR;
 vtputs(buf);
    return(l);
}

int htprintf(fmt)
unsigned char *fmt;
{
va_list argptr;
unsigned char buf[BUFSIZ];
int l;

 va_start(argptr,fmt);
   l= vsprintf(buf,fmt,argptr);
    va_end(argptr);
 attr= REV_ATTR;
 vtputs(buf);
    attr= DEF_ATTR;
 return(l);
}
#endif

void cursav(void)
{
     sposx = posx;
   sposy = posy;
}

void curlod(void)
{
        locate(sposx + 1, sposy + 1);
   posx = sposx;
   posy = sposy;
}

void up(register int n)
{
  if(!n) n= 1;
    n += posy + 1;
  locate(posx + 1, n);
}

void dwn(register int n)
{
  if(!n) n= 1;
    n = (posy >= n)? posy - n: 0;
   locate(posx + 1, n + 1);
}

void right(register int n)
{
    if(!n) n= 1;
    n += posx + 1;
  locate(n, posy + 1);
}

void left(register short n)
{
       if(!n) n= 1;
    n = (posx >= n)? posx - n: 0;
   locate(n + 1, posy + 1);
}

void curoff(void)
{
#ifdef __386__
       ir.ECX = 0x2000;
        ir.EAX = 0x100;
 intr86(0x10,&ir, &or);
#else
     __asm {
         mov     cx, 0x2000
              mov     ah, 1
           int 0x10
        }
#endif
}

void curon(void)
{
#ifdef __386__
 ir.ECX = cursor & 0xffff;
       ir.EAX = 0x100;
 intr86(0x10,&ir, &or);
#else
     __asm {
         mov     cx, cursor
              mov     ah, 1
           int 0x10
        }
#endif
}

void sppcnt(int n,int htflg)
{
register i;

        if(htflg) attr= REV_ATTR;
       for(i =0;i < n;++i) vtputc(' ');
        attr= DEF_ATTR;
}

void pushscrn(void)
{
    size_t vramsiz;
 void __far *vp;

#ifdef __386__
   vramsiz = *((unsigned short __far *)MK_FP(0x40,0x4a)) * (*((unsigned char __far *)MK_FP(0x40,0x84)) + 1) * 2;
#else
      vramsiz = *((unsigned short __far *)0x0040004a) * (*((unsigned char __far *)0x00400084) + 1) * 2;
#endif
 savp = malloc(vramsiz);
 if(!chkmemerr(savp)) return;
    vp = vramptr();
 trsfer(savp, vp, vramsiz);
      batch(NULL,"///");
}

void popscrn(void)
{
size_t vramsiz;
void __far *vp;

    batch("///",NULL);
#ifdef __386__
        vramsiz = *((unsigned short __far *)MK_FP(0x40,0x4a)) * (*((unsigned char __far *)MK_FP(0x40,0x84)) + 1) * 2;
#else
      vramsiz = *((unsigned short __far *)0x0040004a) * (*((unsigned char __far *)0x00400084) + 1) * 2;
#endif
 vp = vramptr();
 trsfer(vp, savp, vramsiz);
      vtflush(vp, vramsiz);
   free(savp);
}

void vtopen(void)
{
#ifdef __386__
    segread(&sr);
#endif
     getcpos(&initx, &inity);
        pushscrn();
#ifdef __386__
       if(!lastdisplay)
                lastdisplay= *((unsigned char __far *) MK_FP(0x40, 0x84));
      if(!displaylen)
         displaylen = *((short __far *) MK_FP(0x40, 0x4a));
      cursor = *((unsigned short __far *) MK_FP(0x40, 0x60));
#else
    if(!lastdisplay)
                lastdisplay= *((unsigned char __far *)0x400084);
        if(!displaylen)
         displaylen = *((unsigned short __far *)0x40004a);
       cursor = *((unsigned short __far *)0x00400060);
#endif
   if(lastdisplay < 24) lastdisplay = 24;
  if(displaylen < 80) displaylen = 80;
    curp = vram = vramptr();
}

void vtclose(void)
{
    popscrn();
      posx = posy = 0;
        locate(initx, inity);
}
