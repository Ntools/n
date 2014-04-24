#include "ed.h"
#ifdef MSDOS
# ifdef _MSC_VER
#  include <search.h>
# endif
# ifdef TEXT98
# include <98key.h>
# else
# define        ESC     0x1b
# endif
#else
# define ESC     0x1b
#endif

#define DELETE        0x7f

extern damy(),bkspc(),compil(),delchar(),upl(),dwnl(),rightl(),shell();
extern leftl(),pageup(),pagedwn(),inson(),fword(),rword(),killbuffer();
extern undoexec(),saveend(),quit(),macro(),wordlwr(),worduper(),linejmp();
extern macrocd(), macrost(), macroed(), rightgo(), leftgo(), readfile();
extern indent(),newline(),rightcut(),leftcut(),linecut(),paste(),fileend();
extern filetop(),search(),rsearch(),argset(),replace(),setdistp(), nxtbuffer();
extern nxtfile(), upsave(), writefile(), chagename(),doscom(),nexterr();
extern markset(), markcut(), markcpy(),filebuf(),markgo(),helpfile();
extern windowop(),windowcl(),chgwindow(),setpara(),readonly(),viewfile();
extern digit(),inputval(),charactor();

redisplay(unsigned short k)
{
    extern refr_flg;

    if(ldup) {
	last= refr_flg= OFF;
	chgwindow();
	display();
    }
    chgwindow();
    inf();
    last= refr_flg= OFF;
    return(NG);
}

struct KEY key[]= {
    { damy,     CTRL('U'),  "times",     "Times"  }, /* TIMES 0 */
    { damy,     ESC     ,   "meta",      "Extend 1" }, /* META(ESC) 1 */
    { damy,     CTRL('X'),  "extend",    "Extend 2" }, /* CTRX 2 */
    { bkspc,    CTRL('H'),  "emacscdel", "Back Space" }, /* BACKSPACE(BSPC) 3 */
    { inson,    META | 'I', "insertmode", "Replace/Insert" }, /* INS key 4 */
    { leftl,    CTRL('B'),  "left",       "Backward char" }, /* LEFTCHAR  5 */
    { rightl,   CTRL('F'),  "right",      "Forward char" }, /* RIGHTCHAR 6 */
    { delchar,  DELETE,     "sdelete",    "Delete char" }, /* DEL 7 */
    { leftgo,   CTRL('A'),  "begline",    "Begining of line" }, /* 8 */
    { rightgo,  CTRL('E'),  "endline",    "End of line" }, /* 9 */
    { rightcut, CTRL('K'),  "rightcut",   "Kill line" }, /* 10 */
    { macro,    CTRL('L'),  "macroexec",  "Execute kbd macro" }, /* Macro exec */
    { macrocd,  CTRX | 'L', "macronum",   "Set kbd macro buffer"}, /* Macro Number */
    { shell,    CTRL('C'),  "shell",      "Shell" },
    { doscom,   CTRX | '!', "runshell",   "Execute command" },
    { setdistp, META | '!', "home",       "Set begining of view" },
    { compil,   META | ESC, "compile",    "Compile" }, /* META META */
    { macrost,  CTRX | '(', "macrostart", "Start kbd macro" }, /* Macro Start */
    { newline,  CTRL('J'),  "linsert",    "New line" },
    { macroed,  CTRX | ')', "macroend",   "End kbd macro" }, /* Macro end */
    { upl,      '\x01e',    "up",         "Backward line" },
    { upl,      CTRL('P'),  "up",         "Forward line" },
    { dwnl,     '\x01f',    "down",       "Next line" },
	{ dwnl,     CTRL('N'),  "down",       "Next line" },
    { pageup,   CTRL('V'),  "ppage",      "Scroll up" },
    { pagedwn,  META | 'V', "mpage",      "Scroll down" },
    { linejmp,  META | 'J', "linejmp",    "Jump to line" },
    { undoexec, CTRX | 'U', "undo",       ""},
    { saveend,  CTRL('Z'),  "exit",       "Save & exit" },
    { quit,CTRX | CTRL('C'),"quit",       "quit" },
    { fword,    META | 'F', "pword",      "Forward word" },
    { rword,    META | 'B', "mword",      "Backward word" },
    { wordlwr,  META | 'L', "lword",      "Change lower case." },
    { worduper, META | 'U', "uword",      "Change upper case." },
    { indent,   CTRL('M'),  "emacsnewl",  "New line indent" },
    { leftcut,  CTRL('D'),  "leftcut",    "Delete to begining of line" },
    { nexterr,  CTRX | 'E', "nexterr",    "Display next error" },
    { linecut,  CTRL('O'),  "ldelete",    "Delete line" },
    { paste,    CTRL('Y'),  "paste",      "Paste" },
    { fileend,  META | '>', "fileend",    "End of buffer" },
    { filetop,  META | '<', "filetop",    "begining of buffer" },
    { search,   CTRL('S'),  "psearch",    "Forward search" },
    { search,   META | 'S', "psearch",    "Forward search" },
    { rsearch,  CTRL('R'),  "msearch",    "Backward search" },
    { replace,  META | 'R', "qreplace",   "Replace word" },
    {killbuffer,CTRX | 'K', "killbuffer", "Kill buffer" },
    { argset,   CTRX | 'I', "search",     "Search word" },
    { readfile, CTRX | 'R', "reflesh",    "Read File" },
    { redisplay,CTRL('Q'),  "redisplay",  "Refresh screen" },
    { nxtfile,  CTRX | 'V', "setfile",    "Open File" },
    { viewfile, CTRX | CTRL('V'),"viewfile","Open Read Only File" },
    { nxtbuffer,CTRX | 'X', "nextbuf",    "Next buffer" },
    { upsave,   CTRX | 'S', "save",       "Save buffer" },
    { writefile,CTRX | 'W', "writefile",  "Save as ..."},
    { chagename,META | 'N', "fnamechage", "Change buffer name" },
    { markset,  CTRL('@'),  "markset",    "Mark set" },
    { markset,  META | '@', "markset",    "Mark set" },
    { markcut,  CTRL('W'),  "cut",        "Kill region" },
    { markcpy,  META | 'W', "copy",       "Copy region" },
    { markgo,   CTRX | CTRL('X'),"mark",  "Exchange point to mark"},
    { filebuf,  CTRX | 'B', "buffernum",  "Switch to buffer" },
    { windowop, CTRX | '2', "setwindow",  "Sprit window" },
    { windowcl, CTRX | '1', "closewindow","Delete other window" },
    { chgwindow,CTRX | 'N', "window",     "Change window" },
    { setpara,  CTRX | CTRL('S'),"setpara","Search word of cursor point" },
    { readonly, CTRX | 'P', "readonly",   "Set Read only" },
    { helpfile, CTRX | CTRL('H'),"help",  "Help" },
    { digit,    CTRX | 'H', "digit",      "Inclimental Digit" },
    { inputval, META | 'H', "inputval",   "Input Digit" },
    { 0, 0 , NULL }
};

#ifdef TEXT98
FUNC_DEF fkey[]= {
	{ ((FUNC1 >> 8) + FUNC_CODE), "FUNC1" },
	{ ((FUNC2 >> 8) + FUNC_CODE), "FUNC2" },
	{ ((FUNC3 >> 8) + FUNC_CODE), "FUNC3" },
	{ ((FUNC4 >> 8) + FUNC_CODE), "FUNC4" },
	{ ((FUNC5 >> 8) + FUNC_CODE), "FUNC5" },
	{ ((FUNC6 >> 8) + FUNC_CODE), "FUNC6" },
	{ ((FUNC7 >> 8) + FUNC_CODE), "FUNC7" },
	{ ((FUNC8 >> 8) + FUNC_CODE), "FUNC8" },
	{ ((FUNC9 >> 8) + FUNC_CODE), "FUNC9" },
	{ ((FUNC10 >> 8) + FUNC_CODE), "FUNC10" },

	{ ((SFUNC1 >> 8) + FUNC_CODE), "SFUNC1" },
	{ ((SFUNC2 >> 8) + FUNC_CODE), "SFUNC2" },
	{ ((SFUNC3 >> 8) + FUNC_CODE), "SFUNC3" },
	{ ((SFUNC4 >> 8) + FUNC_CODE), "SFUNC4" },
	{ ((SFUNC5 >> 8) + FUNC_CODE), "SFUNC5" },
	{ ((SFUNC6 >> 8) + FUNC_CODE), "SFUNC6" },
	{ ((SFUNC7 >> 8) + FUNC_CODE), "SFUNC7" },
	{ ((SFUNC8 >> 8) + FUNC_CODE), "SFUNC8" },
	{ ((SFUNC9 >> 8) + FUNC_CODE), "SFUNC9" },
	{ ((SFUNC10 >> 8) + FUNC_CODE), "SFUNC10" },

	{ ((CFUNC1 >> 8) + FUNC_CODE), "CFUNC1" },
	{ ((CFUNC2 >> 8) + FUNC_CODE), "CFUNC2" },
	{ ((CFUNC3 >> 8) + FUNC_CODE), "CFUNC3" },
	{ ((CFUNC4 >> 8) + FUNC_CODE), "CFUNC4" },
	{ ((CFUNC5 >> 8) + FUNC_CODE), "CFUNC5" },
	{ ((CFUNC6 >> 8) + FUNC_CODE), "CFUNC6" },
	{ ((CFUNC7 >> 8) + FUNC_CODE), "CFUNC7" },
	{ ((CFUNC8 >> 8) + FUNC_CODE), "CFUNC8" },
	{ ((CFUNC9 >> 8) + FUNC_CODE), "CFUNC9" },
	{ ((CFUNC10 >> 8) + FUNC_CODE), "CFUNC10" },

	{ ((ROLLUP >> 8) + FUNC_CODE), "ROLLUP" },
	{ ((ROLLDOWN >> 8) + FUNC_CODE), "ROLLDOWN" },

	{ ((INS >> 8) + FUNC_CODE), "INS" },
	{ ((DEL >> 8) + FUNC_CODE), "DEL" },

	{ ((INS >> 8) + FUNC_CODE + 0x80), "SINS" },
	{ ((DEL >> 8) + FUNC_CODE + 0x80), "SDEL" },

	{ ((UP >> 8) + FUNC_CODE), "UP" },
	{ ((LEFT >> 8) + FUNC_CODE), "LEFT" },
	{ ((RIGHT >> 8) + FUNC_CODE), "RIGHT" },
	{ ((DOWN >> 8) + FUNC_CODE), "DOWN" },

	{ ((UP >> 8) + FUNC_CODE + 0x80), "SUP" },
	{ ((LEFT >> 8) + FUNC_CODE + 0x80), "SLEFT" },
	{ ((RIGHT >> 8) + FUNC_CODE + 0x80), "SRIGHT" },
	{ ((DOWN >> 8) + FUNC_CODE + 0x80), "SDOWN" },

	{ ((CLR >> 8) + FUNC_CODE), "CLR" },
	{ ((HELP >> 8) + FUNC_CODE), "HELP" },
	{ ((HELP >> 8) + FUNC_CODE + 0x80), "SHELP" },

	{ ((HOME >> 8) + FUNC_CODE), "HOME" },
	{ 0x1a + FUNC_CODE, "CTRL@" }
};
#define FKEYSIZ  (sizeof(fkey) / sizeof(FUNC_DEF))
#endif

#ifdef DOSV
FUNC_DEF fkey[]= {
	{ (0x3b + FUNC_CODE), "FUNC1" },
	{ (0x3c + FUNC_CODE), "FUNC2" },
	{ (0x3d + FUNC_CODE), "FUNC3" },
	{ (0x3e + FUNC_CODE), "FUNC4" },
	{ (0x3f + FUNC_CODE), "FUNC5" },
	{ (0x40 + FUNC_CODE), "FUNC6" },
	{ (0x41 + FUNC_CODE), "FUNC7" },
	{ (0x42 + FUNC_CODE), "FUNC8" },
	{ (0x43 + FUNC_CODE), "FUNC9" },
	{ (0x44 + FUNC_CODE), "FUNC10" },

	{ (0x54 + FUNC_CODE), "SFUNC1" },
	{ (0x55 + FUNC_CODE), "SFUNC2" },
	{ (0x56 + FUNC_CODE), "SFUNC3" },
	{ (0x57 + FUNC_CODE), "SFUNC4" },
	{ (0x58 + FUNC_CODE), "SFUNC5" },
	{ (0x59 + FUNC_CODE), "SFUNC6" },
	{ (0x5a + FUNC_CODE), "SFUNC7" },
	{ (0x5b + FUNC_CODE), "SFUNC8" },
	{ (0x5c + FUNC_CODE), "SFUNC9" },
	{ (0x5d + FUNC_CODE), "SFUNC10" },

	{ (0x5e + FUNC_CODE), "CFUNC1" },
	{ (0x5f + FUNC_CODE), "CFUNC2" },
	{ (0x60 + FUNC_CODE), "CFUNC3" },
	{ (0x61 + FUNC_CODE), "CFUNC4" },
	{ (0x62 + FUNC_CODE), "CFUNC5" },
	{ (0x63 + FUNC_CODE), "CFUNC6" },
	{ (0x64 + FUNC_CODE), "CFUNC7" },
	{ (0x65 + FUNC_CODE), "CFUNC8" },
	{ (0x66 + FUNC_CODE), "CFUNC9" },
	{ (0x67 + FUNC_CODE), "CFUNC10" },

	{ (0x68 + FUNC_CODE), "AFUNC1" },
	{ (0x69 + FUNC_CODE), "AFUNC2" },
	{ (0x6a + FUNC_CODE), "AFUNC3" },
	{ (0x6b + FUNC_CODE), "AFUNC4" },
	{ (0x6c + FUNC_CODE), "AFUNC5" },
	{ (0x6d + FUNC_CODE), "AFUNC6" },
	{ (0x6e + FUNC_CODE), "AFUNC7" },
	{ (0x6f + FUNC_CODE), "AFUNC8" },
	{ (0x70 + FUNC_CODE), "AFUNC9" },
	{ (0x71 + FUNC_CODE), "AFUNC10" },

	{ (0x49 + FUNC_CODE), "PAGEUP" },
	{ (0x51 + FUNC_CODE), "PAGEDOWN" },

	{ (0x84 + FUNC_CODE), "CPAGEUP" },
	{ (0x76 + FUNC_CODE), "CPAGEDOWN" },

	{ (0x52 + FUNC_CODE), "INSERT" },
	{ (0x52 + FUNC_CODE), "INS" },
	{ (0x53 + FUNC_CODE), "DELETE" },
	{ (0x53 + FUNC_CODE), "DEL" },

	{ (0x48 + FUNC_CODE), "UP" },
	{ (0x4b + FUNC_CODE), "LEFT" },
	{ (0x4d + FUNC_CODE), "RIGHT" },
	{ (0x50 + FUNC_CODE), "DOWN" },

	{ (0x73 + FUNC_CODE), "CLEFT" },
	{ (0x74 + FUNC_CODE), "CRIGHT" },

	{ (0x47 + FUNC_CODE), "HOME" },
	{ (0x4f + FUNC_CODE), "END" },
	{ (0x77 + FUNC_CODE), "CHOME" },
	{ (0x75 + FUNC_CODE), "CEND" },

	{ (0x1e + FUNC_CODE), "ALT_A" },
	{ (0x30 + FUNC_CODE), "ALT_B" },
	{ (0x2e + FUNC_CODE), "ALT_C" },
	{ (0x20 + FUNC_CODE), "ALT_D" },
	{ (0x12 + FUNC_CODE), "ALT_E" },
	{ (0x21 + FUNC_CODE), "ALT_F" },
	{ (0x22 + FUNC_CODE), "ALT_G" },
	{ (0x23 + FUNC_CODE), "ALT_H" },
	{ (0x17 + FUNC_CODE), "ALT_I" },
	{ (0x24 + FUNC_CODE), "ALT_J" },
	{ (0x25 + FUNC_CODE), "ALT_K" },
	{ (0x26 + FUNC_CODE), "ALT_L" },
	{ (0x32 + FUNC_CODE), "ALT_M" },
	{ (0x31 + FUNC_CODE), "ALT_N" },
	{ (0x18 + FUNC_CODE), "ALT_O" },
	{ (0x19 + FUNC_CODE), "ALT_P" },
	{ (0x10 + FUNC_CODE), "ALT_Q" },
	{ (0x13 + FUNC_CODE), "ALT_R" },
	{ (0x1f + FUNC_CODE), "ALT_S" },
	{ (0x14 + FUNC_CODE), "ALT_T" },
	{ (0x2f + FUNC_CODE), "ALT_V" },
	{ (0x11 + FUNC_CODE), "ALT_W" },
	{ (0x2d + FUNC_CODE), "ALT_X" },
	{ (0x15 + FUNC_CODE), "ALT_Y" },
	{ (0x2c + FUNC_CODE), "ALT_Z" }
};
#define FKEYSIZ        (sizeof(fkey) / sizeof(FUNC_DEF))
#endif

#if (defined DOSV) || (defined TEXT98) || (defined UNIX)

/*------------------ UNIX & DOSV & PC98 ------------------*/
initdefkey(void)
{
	struct KEY kk,*kp;
	int i;

	for(i= 0;*defkey[i].keyword != '\0';++i) {
		kk.keyword = defkey[i].keyword;
		kp = lfind(&kk, keyptr, &key_count,sizeof(struct KEY),cpkey);
		if(kp == NULL) {
			ndprintf("Systen error !!\a\n");
			exit(5);
		}
		if(i == 7) defkey[i].code = kp->code = 0x7f;
		else defkey[i].code = kp->code;
	}
	return(OK);
}

# ifdef DOSV
/*------------------ DOSV ONLY ------------------*/
unsigned funckeychk(unsigned c)
{
	register i;
	unsigned cd;

	for(i = 0;i < FKEYSIZ;++i)
		if((fkey[i].code & 0xff) == c) return(fkey[i].code);
	return(0);
}
# endif
/*-----------------------------------------------*/

int funcmp(const void *lp1, const void *lp2)
{
	return(stricmp(((FUNC_DEF *)lp1)->keyword,((FUNC_DEF *)lp2)->keyword));
}

# if (defined DOSV) || (defined TEXT98)
/*------------------ DOSV & PC98 ONLY ------------------*/
unsigned short chkfunckey(unsigned char *p)
{
	unsigned short cd;
	FUNC_DEF *lp,k;
	static unsigned char buf[FILENAME_MAX];

	for(cd= 0;*p > ' ';++p,++cd) buf[cd]= *p;
	buf[cd]= '\0';
	k.keyword= buf;
	k.code= 0;
	cd = FKEYSIZ;
	lp = lfind(&k,fkey,&cd,sizeof(FUNC_DEF),funcmp);
	return((lp == NULL)? 0:lp->code);
}

void funcwrite(unsigned short cd,FILE *fp)
{
	register i;

	for(i= 0;i < FKEYSIZ;++i) {
		if(cd == fkey[i].code) {
			fprintf(fp,"%s\n",fkey[i].keyword);
			return;
		}
	}
}

void funcbuffer(unsigned short cd,unsigned char *buf)
{
	register i;

	for(i= 0;i < FKEYSIZ;++i) {
		if(cd == fkey[i].code) {
			strcat(buf,fkey[i].keyword);
			return;
		}
	}
}
# endif
#endif
