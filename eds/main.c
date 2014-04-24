#define MAIN 1
#ifdef MSDOS
#include      <conio.h>
#endif
#include "ed.h"

FILE *out_fp= NULL;
int back_flg= OFF;
int tabset= OFF;
int execinitbl= OFF;
int beut_flg;
int bat_flg = ON;
int arg_flg = OFF;

#ifdef __386__
# define VERSION   "6.00"
unsigned short stack_ss;
unsigned short stack_sp;
#endif

extern char *tpnam;

#ifdef TEXT98

void pushscrn(void);
void popscrn(void);

#endif

int main(int argc,char *argv[])
{
	register int i;
	int st, read_only;
	char *sp;
	extern char *ininame;
	int batch(char *,char *);
	void help(int);

	st= 0;
#ifdef __386__
	fprintf(stderr, "Nobby's Editor Version %s\n"
	"Copyright (c) Nobby N. Hirano 1990-94\n", VERSION);
#endif
#ifdef TEXT98
	lastdisplay= 24;
	displaylen = 80;
#endif
#ifdef DOSV
	lastdisplay= 0;
	displaylen = 0;
#endif
	read_only = beut_flg = OFF;
	meminit();
	keycount();
	tool();
	initdefkey();
	for(i=1;i < argc;++i) {
		sp= argv[i];
		if(*sp == '-') {
			sp= argv[i]+1;
			while(*sp != '\0') {
				switch(toupper(*sp)) {          /* to lower case */
					case 'B' : back_flg= ON; ++sp; break;
					case 'E' : bat_flg= OFF; ++sp; break;
#if (defined TEXT98)
					case 'H' : beut_flg= ON; ++sp; break;
					case 'N' : beut_flg= OFF; ++sp; break;
#endif
					case 'L' :
					++sp;
					if(*sp == '=') ++sp;
					if((lastdisplay= atoi(sp)) != 0) {
						while(isdigit(*sp)) ++sp;
						break;
					}
					fprintf(stderr,"Unknow option '%s' !\n",sp);
					help(1);
					case 'V' :
					fprintf(stderr,"Copyright (c) Nobby N. Hirano 1990 - 94 All rights reserved.\r\n"
#if (defined TEXT98)
					"Version 5.0  %s  for PC9801 & PC-286,386 i80286,386,486.\n"
#elif (defined ATVRAM)
# ifdef __386__
					"Version %s %s  for PC-DOS MS-DOS J5.0/V, J6.x/V DPMI Version.\n", VERSION
# else
					"Version 6.0  %s  for PC-DOS MS-DOS J5.0/V, J6.x/V.\n"
# endif
#elif (defined DOSV)
					"Version 5.0  %s  for PC-AT, its Compatible and J3100.\n"
#elif (defined UNIX)
					"Version 1.1.0  %s  for UNIX.\r\n"
#endif
					,__DATE__);
					exit(1);
					case 'M' :
					++sp;
					execinitbl= ON;
					break;
					case 'R' :
					++sp;
					read_only = ON;
					break;
					case '?':
					help(0);
					case 'S' :
					++sp;
					if(*sp == '=') ++sp;
					if((tabs= atoi(sp)) != 0) {
						tabset= ON;
						while(isdigit(*sp)) ++sp;
						break;
					}
					default :
					fprintf(stderr,"Unknow option '%s' !\n",sp);
					help(1);
				}
			}
		}
		else {
			++st;
			arg_flg = ON;
		}
	}
	if(execinitbl) {
		printf("Write Key command tablefile to [keys.ini].\n");
		toolwrite("keys.ini");
		exit(0);
	}
#ifdef UNIX
	system("stty -icanon -echo intr ^T");
	ttopen();
#endif
#if defined(TEXT98)
	pushscrn();
#elif !defined(ATVRAM) && !defined(UNIX)
	batch(NULL,"///");
#endif
#if defined(ATVRAM)
	vtopen();
#elif defined(DOSV)
	if(!lastdisplay) {
	lastdisplay= *((unsigned char __far *)
# ifdef __386__
	MK_FP(0x40, 0x84));
# else
	0x400084);
#endif
	if(lastdisplay < 24) lastdisplay = 24;
}
	if(!displaylen) {
	displaylen = *((short __far *)
# ifdef __386__
	MK_FP(0x40, 0x4a));
# else
	0x40004a);
#endif
	if(displaylen < 80) displaylen = 80;
}
#endif
	line= NULL;
	cls();
	curon();
#ifdef MSDOS
	if(!st) edmain("*.*", read_only);
#else
	if(!st) edmain("main", read_only);
#endif
	else {
	for(i=1;i < argc;++i) {
	if(*argv[i] != '-') {
	edmain(argv[i], read_only);
	break;
}
}
}
	locate(1,lastdisplay); vtputc('\n');
	if((sp= (char *)fopen(tpnam,"r")) != NULL) {
	fclose((FILE *)sp);
	unlink(tpnam);
}
#if defined(TEXT98)
	popscrn();
#elif defined(ATVRAM)
	vtclose();
#elif defined(UNIX)
	ttclose();
	system("stty icanon echo intr ^C");
#else
	st= batch("///",NULL);
#endif
	exit(0);
}

#ifndef UNIX
int batch(char *st,char *ed)
{
char buf[BUFSIZ],*p,*e,*setpsnt(char *);
FILE *fp;
int s,sts;

#ifdef DBG
	return OK;
#endif

	if(!bat_flg) return OK;
	if((e= getenv("BAT")) == NULL) e= "edit.ext";
	if((fp= fopen(e,"r")) == NULL) {
		_searchenv(e,"PATH",buf);
		if(buf[0] == '\0') return(NG);
		if((fp= fopen(buf,"r")) == NULL) {
			msg("Execute file [%s] is not found !!",e);
			return(NG);
		}
	}
	s= OK;
	while(fgets(buf,BUFSIZ,fp) != NULL) {
		crcut(buf);
		if(st != NULL) {
			if(!strcmpi(st,buf)) {
				st= NULL;
			}
			continue;
		}
		if(ed != NULL && !strcmpi(ed,buf)) break;
		for(p= buf;*p <= ' ' && *p != '\0';++p) ;
		if(*p == '\0') continue;
		e= setpsnt(p);
		if((sts= system(e)) != 0) {
			msg("Error command '%s' errorlevel[%d]",buf,sts);
			s= NG;
		}
	}
	fclose(fp);
	return(s);
}
#endif

char *setpsnt(char *s)
{
static char buf[BUFSIZ];
char *p1;

	if((p1= strchr(s,'%')) == NULL) return(s);
	sprintf(buf,s,editfile[edfile].filename);
	return(buf);
}

void help(int ern)
{
	fprintf(stderr,"Syntax:      n [<option>] <filename> [<option>]\n");
	fprintf(stderr,"Function:    multifile screen editor.\n");
	fprintf(stderr,"Option:      -l=<num>    set max line <num>.(if not 25lines display\n");
	fprintf(stderr,"             -m          write key code table file,don't execute editor.\n");
	fprintf(stderr,"             -n          normal character mode.\n");
	fprintf(stderr,"             -h          half size kanji character mode.\n");
	fprintf(stderr,"             -r          open read only <filename>.\n");
	fprintf(stderr,"             -b          create bakupfile.\n");
	fprintf(stderr,"             -e          don't execute 'edit.ext'.\n");
	fprintf(stderr,"             -s=<num>    tabset to <num>.\n");
	exit(ern);
}
