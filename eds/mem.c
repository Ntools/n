#include "ed.h"
#include <signal.h>
#include <fcntl.h>

#define intems(c1,c2)       int86(0x67,(c1),(c2))
#define intemsx(c1,c2,ss)  int86x(0x67,(c1),(c2),(ss))

unsigned int cutcnt;
unsigned int alpg,ttlpg;
unsigned int handle= 0;
unsigned char _far *emsbase;
int emsflag= OFF;
int filenum= 0xfff;
unsigned long emssize;
unsigned char *tpnam;
SCRN *markcp = NULL;

struct MARK smark= { NULL, 0 ,0};

#ifdef MSDOS
union REGS inregs;
union REGS outregs;
struct SREGS segregs;

/* Handles SIGINT (Ctrl+C) interrupt. */
void ctrlc( int sig )
{
	signal(SIGINT,SIG_IGN);
	signal(SIGINT,ctrlc);
}
#else
extern errno;
#endif

meminit(void)
{
	int i;
	unsigned char *p;

	if((tpnam = tmpnam(NULL)) == NULL) {
		fprintf(stderr, "Can't make temp file.\a\n");
		exit(1);
	}
	if((macbuffer= (unsigned short **)calloc(MACROMAX,sizeof(unsigned short *))) == NULL) {
		perror("Not enogh memory !!");
		exit(1);
	}
	for(i=0;i < MACROMAX;++i) {
		if((macbuffer[i]= (unsigned short *)calloc(MACROBUF,sizeof(unsigned short))) == NULL) {
			perror("Not enogh memory !!");
			exit(1);
		}
		*macbuffer[i]= ENDMARK;
	}
	if((cutbuf= (unsigned char *)calloc(1,sizeof(char))) == NULL) {
		perror("Not enogh memory !!");
		exit(1);
	}
	*cutbuf= '\0';
#ifdef MSDOS
	if(signal(SIGINT,ctrlc) == SIG_ERR) {
		fprintf(stderr,"Can't set SIGINT !!");
		abort();
	}
# ifdef EMS98
	if(emschk() == OK) emsset();
}

emsset()
{
	inregs.h.ah= 0x40;
	intems(&inregs,&outregs); if(outregs.h.ah) return(NG);
	inregs.h.ah= 0x42;
	intems(&inregs,&outregs); if(outregs.h.ah) return(NG);
	alpg= outregs.x.bx; ttlpg= outregs.x.dx;
	if(alpg < 4) return(NG); emssize= alpg*16L*1024L;
	inregs.x.bx= alpg; inregs.h.ah= 0x43;
	intems(&inregs,&outregs); if(outregs.h.ah) return(NG);
	handle= outregs.x.dx;
	inregs.h.ah= 0x41;
	intems(&inregs,&outregs); if(outregs.h.ah) return(NG);
	emsbase= (unsigned char far *)((unsigned long)outregs.x.bx*0x10000L);
	emsflag= ON;
	inregs.h.ah= 0x44; outregs.h.ah= 0;
	inregs.x.bx= 0; emssize= 0L; inregs.x.dx= handle;
	for(inregs.h.al= 0;outregs.h.ah == 0;++inregs.h.al,++inregs.x.bx)
		intems(&inregs,&outregs);
	if(outregs.h.ah != 0x8b) return(NG);
	emssize= 0x4000L*(long)(inregs.h.al-1);
}

emsmap()
{
	static unsigned logpg= 0;

	inregs.x.dx= handle;
}

emsfree(void)
{
	inregs.h.ah= 0x45;
	inregs.x.dx= handle;
	intems(&inregs,&outregs); if(outregs.h.ah) return(NG);
	return(OK);
# endif
#endif
}

#ifdef MSDOS
void strfree(unsigned char *p)
{
	if(emsflag) free(p);
	else free(p);
}

void linefree(SCRN *lp)
{
	if(emsflag) free(lp);
	else free(lp);
}
#endif

chkmemerr(void *p)
{
	if(p == NULL) {
		msg("Not enogh memory !![%s]",strerror(errno));
		memerr= NG;
	}
	else memerr= OK;
	return(memerr);
}

SCRN *memgetl(SCRN *op)
{
SCRN *lp;

	lp= malloc(sizeof(SCRN));
	if(chkmemerr(lp) == OK) {
		lp->rev= op;
		lp->fwd= NULL;
	}
	if(op != NULL) op->fwd= lp;
	else line= lp;
	return(lp);
}

unsigned char *memgets(unsigned char *buf)
{
	unsigned char *pt;

	pt= strdup(buf);
	chkmemerr(pt);
	return(pt);
}

SCRN *clearmem(SCRN *lp,int cnt)
{
register unsigned i;
SCRN *cp;

	if(cnt == EOF) { i= 0; --i; }
	else i= cnt;
	while(i-- && lp != (SCRN *)NULL) {
		strfree(lp->buffer);
		cp= lp;
		lp= lp->fwd;
		linefree(cp);
	}
	return(lp);
}

int cutcpy(unsigned char *p)
{
unsigned char *np;
SCRN *lp;

	if((np= (unsigned char *)malloc(strlen(p)+1)) == NULL) {
		msg("Not enogh memory !![%s]",strerror(errno));
		memerr= NG;
		cutbuf = "";
	}
	else {
		memerr= OK;
		if(cutbuf != NULL) free(cutbuf);
		cutbuf= np;
		strcpy(np,p);
	}
	return(memerr);
}

int cutcat(unsigned char *p)
{
	unsigned char *np;
	size_t l;

	l= strlen(p)+strlen(cutbuf)+1;
	if((np= (unsigned char *)malloc(l)) == NULL) {
		msg("Not enogh memory !![%s]",strerror(errno));
		memerr= NG;
		*cutbuf= '\0';
	}
	else {
		memerr= OK;
		strcpy(np,cutbuf);
		free(cutbuf);
		strcat(np,p);
		cutbuf= np;
	}
	return(memerr);
}

int markgo(void)
{
	if(filenum != edfile) {
		msg("No mark !");
		return(NG);
	}
	if(smark.line != NULL && line == mark.line) {
		line= smark.line;
		curposx= smark.posx;
		msg("Curent line");
	}
	else {
		msg("Mark line");
		smark.line= line;
		smark.posx= curposx;
		line= mark.line;
		curposx= mark.posx;
	}
	curposy= disset(5);
	return(OK);
}

int markset(void)
{
	filenum= edfile;
	mark.line= line;
	mark.num = line->num;
	mark.posx= curposx;
	smark.line= NULL;
	msg("Mark set");
	return(OK);
}

SCRN *search_mark()
{
	SCRN *lp;
	unsigned i;

	lp = editfile[edfile].topline;
	for(i = 1;mark.num != lp->num;lp = lp->fwd) if(lp == NULL) break;
	if(lp != NULL) {
		i = strlen(lp->buffer);
		if(mark.posx > i) mark.posx = i;
	}
	return(lp);
}

int markcpy(void)
{
	unsigned char *p,buf[BUFSIZ];
	SCRN *lp;
	int i;
	extern posflag;

	posflag= ON;
	if((lp = search_mark()) == NULL) {
		msg("Can't store to buffer !!");
		return(NG);
	}
	if(lp->num > line->num) {
		msg("Illegal oparation");
		return(NG);
	}
	markcp = lp;
	p = &lp->buffer[mark.posx];
	cutcnt= 0;
	cutcpy("");
	while(memerr == OK) {
		for(i = 0;*p != '\0' && p != &line->buffer[curposx];++i,++p) buf[i] = *p;
		if(p != &line->buffer[curposx]) buf[i++] = '\n';
		buf[i] = '\0';
		cutcat(buf); ++cutcnt;
		if(lp == line) break;
		lp= lp->fwd; p= lp->buffer;
		if(lp == NULL) {
			msg("Can't store to buffer !!");
			return(NG);
		}
	}
	return(NG);
}

int markcut(void)
{
	unsigned char *p, buf[BUFSIZ];
	SCRN *op,*lp;
	int ox, i;
	extern refr_flg;

	markcpy();
	ox = curposx;
	lp = markcp;
	if(lp == line) {
		for(i = 0;i < mark.posx;++i) buf[i] = lp->buffer[i];
		while(lp->buffer[ox]) buf[i++] = lp->buffer[ox++];
		buf[i] = '\0';
		curposx = mark.posx;
		update(buf, 0);
	}
	else if(lp->num < line->num) {
		lp->buffer[mark.posx] = '\0';
		p = &line->buffer[curposx];
		sprintf(buf, "%s%s", lp->buffer, p);
		line->rev = lp->rev;
		lp->rev->fwd = line;
		while(lp != line) {
			free(lp->buffer);
			op = lp;
			lp = lp->fwd;
			free(op);
		}
		free(line->buffer);
		line->buffer = memgets(buf);
		curposx = mark.posx;
		curposy = disset(5);
		refr_flg = last = OFF;
	}
	return(NG);
}

#ifdef MSDOS
# ifdef EMS98
emschk(void)
{
	int hdl;
	unsigned s;

	if(_dos_open("EMMXXXX0",O_RDONLY,&hdl) != 0) return(NG);
	inregs.x.ax= 0x4400;
	inregs.x.bx= hdl;
	intdos(&inregs,&outregs);
	if(outregs.x.cflag && !(outregs.x.dx & 0x80)) return(NG);
	inregs.x.ax= 0x4407;
	inregs.x.bx= hdl;
	intdos(&inregs,&outregs);
	if(outregs.x.cflag && !(outregs.x.dx & 0x80)) return(NG);
	_dos_close(hdl);
	if(outregs.h.al != 0xff) return(NG);
	return(OK);
}
# endif
#endif

linenum(void)
{
	SCRN *lp;
	register unsigned int n;

	lp= editfile[edfile].topline;
	n= 0;
	while(lp != NULL) {
		lp->num= ++n;
		lp= lp->fwd;
	}
	return(OK);
}
