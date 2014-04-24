#include	<stdio.h>
#include	<string.h>
#include	<time.h>

#ifdef MSDOS
# include	<conio.h>
# include	<stdlib.h>
# include	<malloc.h>
# include	<dos.h>
#endif

#if (defined __386__)
# include	<bios.h>
# define	_ffree          free
# define    _fmalloc        malloc
#endif

#ifdef UNIX
# ifdef LINUX
#  include	<slcurses.h>

#define KEY_DC		SL_KEY_DELETE		/* Delete character */
#define KEY_IC		SL_KEY_IC			/* Insert char or enter insert mode */
#define KEY_EOL         0517            /* Clear to end of line */


# else
#  include	<curses.h>
# endif
# define	ESC			0x1b
# define	_far
# define	__far
# define	_ffree		free
# define	_fmalloc	malloc
# endif

#define		CTRL_(c)		((c)-'@')

unsigned ungetdata= 0;
unsigned getdata= 0;

time_t   st_time = (time_t)0;

unsigned short ktbl[] = {
/*    F1     F2     F3     F4     F5     F6     F7     F8     F9    F10    */
	0x2062,0x2063,0x2064,0x2065,0x2066,0x2067,0x2068,0x2069,0x206a,0x206b,
	0x2082,0x2083,0x2084,0x2085,0x2086,0x2087,0x2088,0x2089,0x208a,0x208b,
	0x2036,0x2037,0x2038,0x2039,0x203a,0x203b,
	0x203c,0x203d,0x203e,0x203f,0x20ae,
	0x2092,0x2093,0x2094,0x2095,0x2096,0x2097,0x2098,0x2099,0x209a,0x209b
};
#ifdef DOSV

# define      KEY_BIOS        0x16
# define    SHIFT           3

#endif

#ifdef TEXT98

# define    KEY_BIOS        0x18
# define    EXTEND98        0xdc

typedef struct {
	char f1[16],f2[16],f3[16],f4[16],f5[16];
	char f6[16],f7[16],f8[16],f9[16],f10[16];
	char sf1[16],sf2[16],sf3[16],sf4[16],sf5[16];
	char sf6[16],sf7[16],sf8[16],sf9[16],sf10[16];
	char rollup[6],rolldown[6],ins[6], del[6];
	char up[6],left[6],right[6],down[6],clr[6],help[6],home[6];
	char cf1[16],cf2[16],cf3[16],cf4[16],cf5[16];
	char cf6[16],cf7[16],cf8[16],cf9[16],cf10[16];
} FKEY;

FKEY _far *funckey;

void funcget(unsigned mode,void _far *fkey)
{
	union REGS      ir, or;
	struct SREGS    sr;

	sr.ds = FP_SEG(fkey);
	ir.x.dx = FP_OFF(fkey);
	ir.x.ax = mode;
	ir.h.cl = 0x0c;
	int86x(EXTEND98, &ir, &or, &sr);
}

void funcset(unsigned mode,void _far *fkey)
{
	union REGS ir, or;
	struct SREGS     sr;

	sr.ds = FP_SEG(fkey);
	ir.x.dx = FP_OFF(fkey);
	ir.x.ax = mode;
	ir.h.cl = 0x0d;
	int86x(EXTEND98, &ir, &or, &sr);
}

void getfkey(void)
{
	register i;
	char _far *p;
	union REGS  ir, or;

	if((funckey = _fmalloc(sizeof(FKEY))) == NULL) {
		fprintf(stderr,"Memory allcate Error !");
		exit(2);
	}
	ir.h.cl = 0x0f;         /* Avairable CTRL FUNC KEY */
	ir.x.ax = 0;
	int86(EXTEND98, &ir, &or);
	funcget(0,funckey);
	for(p= funckey->cf1,i= 0x2a;i < 0x34;++i, p += 16)
		funcget(i,p);
}

void setfkey(void)
{
	unsigned s,o;
	char _far *p;
	FKEY _far *fkey;

	if((fkey = _fmalloc(sizeof(FKEY))) == NULL) {
		fprintf(stderr,"Memory allcate Error !");
		exit(2);
	}
	p= fkey->f1;
	for(s=0;s < 20;++s)
		for(o = 0;o < 16;++o,++p) *p= '\0';
	p= fkey->f1;
	for(s=0;s < 20;++s) {
		*p = 0xff;
		*(p + 1) = (s + 1);
		p += 16;
	}
	p= fkey->rollup;
	for(s=0;s < 11;++s)
		for(o = 0;o < 6;++o,++p) *p= '\0';
	p= fkey->rollup;
	for(s=0;s < 11;++s) {
		*p = 0xff;
		*(p + 1) = (s + 21);
		p += 6;
	}
	p = fkey->cf1;
	for(s= 0;s < 10;++s)
		for(o = 0;o < 16;++o,++p) *p= '\0';
	p = fkey->cf1;
	for(s= 0;s < 10;++s) {
		*p = 0xff;
		*(p + 1) = (s + 32);
		p += 16;
	}
	funcset(0,fkey);
	for(p= fkey->cf1,s = 0x2a;s < 0x34;++s, p += 16)
		funcset(s,p);
}

void initfkey(void)
{
	getfkey();
	setfkey();
}

void backfkey(void)
{
	char _far *p;
	register i;

	funcset(0,funckey);
	for(p= funckey->cf1,i= 0x2a;i < 0x34;++i, p += 16) funcset(i,p);
}

shiftchk(void)
{
union REGS      ir, or;

	ir.h.ah = 2;
	int86(0x18, &ir, &or);
	return( or.x.ax & 1);
}

int keysts(void)
{
	__asm {
		mov     ah, 1
		int     0x18
		xor     ah, ah
		mov     al, bh
	}
}

#endif /* END TEXT98 */

void ungetkey(void)
{
	if(!ungetdata) ungetdata= getdata;
}

#ifdef DOSV

# if (defined __386__)
kb_sts()
{
	return(_bios_keybrd(_KEYBRD_READY));
}

unsigned kb_get(void)
{
	return(_bios_keybrd(_KEYBRD_READ));
}

unsigned getkeycode(void)
{
	while(_bios_keybrd(_KEYBRD_READY)) _bios_keybrd(_KEYBRD_READ);
	return(_bios_keybrd(_KEYBRD_READ));
}

# else

kb_sts()
{
	__asm {
		mov     ah, 0x1
		int     0x16
		mov     ax, 1
		jnz     ksts01
		xor     al, al
		ksts01:
	}
}

unsigned kb_get(void)
{
	__asm {
		xor     ah, ah
		int     0x16
	}
}

unsigned getkeycode(void)
{
	__asm {
	  gkc01:
		mov     ah, 0x1
		int     0x16
		jz      gkc02
		xor     ah, ah
		int     0x16
		jp      gkc01

	  gkc02:
		xor     ah, ah
		int     0x16
	}
}
# endif

int keysts(void)
{
	unsigned kc, funckeychk();

	if(!kb_sts()) return(0);

	kc = kb_get();
	if(!(kc & 0xff)) {
		kc >>= 8;
		kc = funckeychk(kc);
	}
	else kc &= 0xff;
	ungetdata = kc;
	return 1;
}

unsigned AtKey(void)
{
	unsigned c,funckeychk();

	c = getkeycode();
	if(!(c & 0xff)) {
		c >>= 8;
		c = funckeychk(c);
	}
	else c &= 0xff;
	return c;
}
#endif

unsigned getkey(void)
{
	int k;
	unsigned c;
#ifdef UNIX
	unsigned d, getVkey();
#endif
	extern lastdisplay;

	if(ungetdata) c = ungetdata;
	else {
#ifdef MSDOS
# ifdef DOSV
		c = AtKey();
# else
		c= KEYB();
# endif
#endif
#ifdef UNIX
		c = getVkey();
		if(c != ESC) {
			getdata = c;
			return(c);
		}
		d = getch();
		if(d == '[') {
			d = getch();
			c = getch();
			if(c == '~' && d == '2') c = KEY_IC;
			else if(c == '~' && d == '6') c = KEY_NPAGE;
			else if(c == '~' && d == '5') c = KEY_PPAGE;
			else c = 0x7ff;		/* Unknown code */
		}
		else if(d == 'O') {
			d = getch();
			if(d == CTRL_('@')) c = KEY_HOME;
			else if(d == CTRL_('P')) c = KEY_EOL;
			else c = 0x7ff;		/* Unknown code */
		}
		else {
			ungetdata = d;
			getdata = c;
			return(c);
		}
#endif
#ifdef TEXT98
		if(c == 0xff) {
			k= KEYB();
			--k;
			c= ktbl[k];
			if(k < 32 && k >= 20 && shiftchk()) c |= 0x80;
		}
		else if(!c) c= 0x201a;
#endif
	}
	ungetdata= 0;
	getdata = c;
	return(c);
}

void start_time(void)
{
	time(&st_time);
}

int chk_time(int ovt)
{
	time_t t1, t2;

	t1 = st_time + ovt;
	time(&t2);
	return((t2 < t1)? 0:1);
}
