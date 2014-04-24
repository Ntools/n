#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef __TURBOC__
#include <alloc.h>
#else
#include <malloc.h>
#endif /* End __TURBOC__ */
#include <stdarg.h>

#ifdef MSDOS
#include <process.h>
#include <dos.h>

#else

#ifndef UNIX
#define UNIX
#endif

#ifdef SJIS
# define iskanji(c) ((((c)>=0x81)&&((c)<=0x9f))||(((c)>=0xe0)&&((c)<=0xfc)))
# define iskanji2(c) ((((c)>=0x40)&&((c)<=0x7e))||(((c)>=0x80)&&((c)<=0xfc)))
#else
# define iskanji(c) ((c)&0x80)
# define iskanji2(c) ((c)&0x80)
#endif

void *lsearch(const void *, void *, unsigned int *, unsigned int, int (*)());
void *lfind(const void *, const void *, unsigned int *, unsigned int, int (*)());

#define strcmpi    stricmp
#endif /* End MSDOS */

#define    	SCRN_BUF        192
#define  	SCRN_LEN        192

#define KNJ 1

#define ON     1
#define OK     1
#define OFF    0
#define YES    1
#define NO     0
#define NG     0
#define CTRL(c) ((c)-'@')

#define             K11_TYPE        0x11
#define             K12_TYPE        0x12
#define             K13_TYPE        0x13
#define             K14_TYPE        0x14

#ifdef UNIX
#define  _far
#define  __far
#endif

#ifdef UNIX
#define   _far
#define   __far
#define   cputs(s)        fputs(stdout,(s))
#define   cprintf         printf
#define   _ffree          free
#define   _fmalloc        malloc
#endif

#define MAXFILE       10
#define UNDOLEVEL     10
#define MACROBUF      256
#define MACROMAX      10
#define ENDMARK       0xfff

#define META              0x8000
#define CTRX              0x4000
#define   FUNC_CODE       0x2000

#define C              0
#define ASM            1
#define MASM           2
#define OTHLNG         7

#define TIMES			defkey[0].code
#define METACHAR  		defkey[1].code
#define EXTEND			defkey[2].code
#define INSON           defkey[4].code
#define BSPC			defkey[3].code
#define LEFTCHAR		defkey[5].code
#define RIGHTCHAR		defkey[6].code
#define DELCHAR         defkey[7].code
#define BEGLINE         defkey[8].code
#define ENDLINE         defkey[9].code
#define RIGHTCUT		defkey[10].code

#define DEFSTR0         "times"
#define DEFSTR1         "meta"
#define DEFSTR2         "extend"
#define DEFSTR3         "emacscdel"
#define DEFSTR4         "insertmode"
#define DEFSTR5         "left"
#define DEFSTR6         "right"
#define DEFSTR7         "sdelete"
#define DEFSTR8         "begline"
#define DEFSTR9         "endline"
#define DEFSTR10        "rightcut"

#ifdef __hp9000s700
#pragma HP_ALIGN HPUX_WORD PUSH
#endif

struct LINE {
	struct LINE *fwd;
	struct LINE *rev;
	unsigned num;
	unsigned char *buffer;
};

typedef struct LINE SCRN;

struct MARK {
	struct LINE *line;
	unsigned num;
	int posx;
};

struct KEY {
	int (*func)();
	unsigned short code;
	unsigned char *keyword;
	unsigned char *helpmsg;
};

struct DEFKEY {
	unsigned short code;
	unsigned char *keyword;
};

struct EDIT {
	struct LINE *topline;
	struct LINE *curline;
	unsigned char *filename;
	int writeflag;
	int rdonl;
};

#if (defined TEXT98) || (defined DOSV) || (defined UNIX)
typedef struct {
	unsigned short code;
	unsigned char *keyword;
} FUNC_DEF;
#endif

#ifdef __hp9000s700
#pragma HP_ALIGN POP
#endif

/* Function Call */
/* void trsfer(void _far *,void _far *,size_t); */
void inf(void);
void display(void);
void initfkey(void);
void backfkey(void);
void vtputs(unsigned char __far *);

void linecopy(SCRN *,SCRN *);
void vtputc(int);
void sppcnt(int,int);
#ifdef MSDOS
void strfree(unsigned char *);
void linefree(SCRN *);
#else
# define strfree(p) free((p))
# define linefree(lp) free((lp))
#endif

unsigned char *inputs(unsigned char *,unsigned char *);
unsigned char *putcrt(unsigned char *);
unsigned char *memgets(unsigned char *);
unsigned char *crcut(unsigned char *);
unsigned char *fonly(const unsigned char *);

#ifndef UNIX
# ifdef __386__
void msg(unsigned char *__format ,...);
void dprintf(const unsigned char *__format , ...);
void htprintf(const unsigned char *__format , ...);
# else
void msg(unsigned char *,...);
int dprintf(const unsigned char * , ...);
int htprintf(const unsigned char * , ...);
# endif
#endif
int logical(const unsigned char *,int);
int phygical(const unsigned char *,int);
int cpkey(const void *, const void *);
int codekey(const void *, const void *);
unsigned getkey(void);
unsigned inputu(unsigned char *,int,unsigned short *);

SCRN *memgetl(SCRN *);
SCRN *clearmem(SCRN *,int);

#ifdef MAIN

/* Value */
int displaylen;
int lastdisplay;
FILE *in_fp;
SCRN *line;                             /* curent */
struct UNDO *undo;
struct EDIT editfile[MAXFILE];
SCRN *distop;
struct MARK mark;
struct KEY *keyptr;
size_t key_count;
unsigned char *cutbuf = NULL;
unsigned short **macbuffer;
int  edfile;
int  ldup= 0;
int  insert= ON;
int  memerr= OK;
int  last= OFF;
int  macroflg= OFF;
int  macronum= 0;
int  macropos= 0;
int  rightshift= 0;
int  curposx= 0;
int  curposy= 0;
int  tabs=0;
int  widbase= 0;
int  k1cnt = 0;
int  k2cnt = 0;
long line_num;

#ifdef MSDOS
unsigned char *copl[]= {
	"cl -Zs %s",
	"xasm %s",
	"masm /z %s",
	"",
	"",
	"",
	"",
	""
};
#endif
#ifdef UNIX
unsigned char *copl[]= {
	"lint -Aa -Y %s",
	"xasm %s",
	"xasm %s",
	"",
	"",
	"",
	"",
	""
};
#endif

int tabtbl[]= { 4, 9, 8, 8, 8, 8, 8, 8 };

struct       DEFKEY  defkey[]= {
      { 0,    DEFSTR0 },
      { 0,    DEFSTR1 },
      { 0,    DEFSTR2 },
      { 0,    DEFSTR3 },
      { 0,    DEFSTR4 },
      { 0,    DEFSTR5 },
      { 0,    DEFSTR6 },
      { 0,    DEFSTR7 },
      { 0,    DEFSTR8 },
      { 0,    DEFSTR9 },
      { 0,    DEFSTR10 },
      { 0xffff,"" }
};

#else

/* Value */
extern int displaylen;
extern int lastdisplay;
extern FILE *in_fp;
extern SCRN *line;
extern struct UNDO *undo;
extern struct EDIT editfile[];
extern SCRN *distop;
extern struct MARK mark;
extern unsigned char *cutbuf;
extern unsigned char *copl[];
extern unsigned short **macbuffer;
extern size_t key_count;
extern int  edfile;
extern int  insert;
extern int  memerr;
extern int  last;
extern int    ldup;
extern int  macroflg;
extern int  macronum;
extern int  macropos;
extern int  rightshift;
extern int  curposx;
extern int  curposy;
extern int  tabs;
extern int  widbase;
extern int  tabtbl[];
extern int  k1cnt;
extern int  k2cnt;
extern long line_num;
extern struct KEY *keyptr;
extern struct DEFKEY defkey[];

#endif
