#include "ed.h"
#ifdef MSDOS
#include <sys\types.h>
#include <sys\stat.h>
#endif

extern int posflag;
extern int crp;
extern int refr_flg;
extern int lup;
extern unsigned short eofchar;

unsigned char *file_name(unsigned char *);

#ifdef UNIX
#define fonly        name_only
#endif

SCRN *smlptr;

SCRN *bsprc(SCRN *lp)
{
	SCRN *op;

	op = lp;
	if (lp->fwd) lp->fwd->rev = lp->rev;
	lp->rev->fwd = lp->fwd;
	strfree(lp->buffer);
	smlptr= lp= lp->rev;
	if(op == distop) distop= lp;
	else {
		if(ldup && curposy > (lastdisplay/2)) ;
		else curposy--;
	}
	if(op != NULL) linefree(op);
	locate(1,curposy+1); rcls();
	return(lp);
}

SCRN *entprc(SCRN *lp,unsigned char *buf)
{
	SCRN *udp;

	if(lp->buffer != NULL) strfree(lp->buffer); rcls();
	lp->buffer= memgets(buf);
	udp= lp->fwd;
	lp->fwd= memgetl(lp);
	if(memerr == OK) {
		if(smlptr == line) smlptr= lp;
		lp= lp->fwd;
		lp->fwd= udp;
		if(udp != NULL) udp->rev= lp;
		else lp->num = lp->rev->num + 1;
		lp->buffer= memgets("");
		++curposy;
	}
	*buf= '\0';     return(lp);
}

int update(unsigned char *p,int px)
{
	unsigned char buf[BUFSIZ];
	unsigned char *p1;
	SCRN *lp;
	extern upflag;
	void set_undo(void);

	if(editfile[edfile].rdonl) {
		msg("\a");
		return(NG);
	}
	posflag = ON;
	curposx += px;
	lp = line; smlptr = line;
	p1 = buf; *p1 = '\0';
	do {
		switch(*p) {
			default: *p1= *p; ++p1; *p1= '\0'; break;
			case '\b': lp = bsprc(lp); p1= buf; *p1= '\0'; lup= ON; break;
			case '\n': lp = entprc(lp,buf); p1= buf; *p1= '\0'; lup= ON; break;
		}
		++p;
	} while (*p != '\0');
	if(lp->buffer != NULL) strfree(lp->buffer);
	lp->buffer= memgets(buf);
	if(lp->fwd == NULL) {
		lp->fwd= memgetl(lp);
		lp->fwd->buffer= memgets("");
	}
	if(memerr == OK && line != lp) {
		line= lp;
		refr_flg= setcury(smlptr) - widbase;
		if(refr_flg == EOF) {
			refr_flg= 0;
			curposy= disset((lastdisplay-widbase)/2);
		}
		last= OFF;
	}
	if(last) {
		locate(1,curposy+1); rcls();
		putcrt(buf);
	}
	if((curposy= setcury(line)) == EOF) {
		refr_flg= 0; curposy= disset(5); last= OFF;
	}
	if(!editfile[edfile].writeflag) {
		editfile[edfile].writeflag= ON;
		inf();
	}
	crp= 0;
	if(memerr != NG) set_undo();
	upflag= ON;
	return(memerr);
}

int setcury(SCRN *lp)
{
	register i,cp;
	SCRN *op;

	op= distop; cp= lastdisplay-1;
	for(i= widbase;i < cp;++i) {
		if(op == lp) return(i);
		else if(op == NULL) break;
		op= op->fwd;
	}
	return(EOF);
}

saveend(void)
{
	int i;

	for(i = 0;i < MAXFILE;++i) {
		if(editfile[i].topline != NULL && editfile[i].writeflag && !editfile[i].rdonl)
			save(editfile[i].topline,editfile[i].filename);
	}
	return(EOF);
}

upsave(void)
{
	if(editfile[edfile].rdonl) {
		msg("\a");
		return(NG);
	}
	if(!editfile[edfile].writeflag) return(NG);
	save(editfile[edfile].topline,editfile[edfile].filename);
	return(NG);
}

writefile(void)
{
int s;

	s= fnchage("Write");
	if(s == OK)
		save(editfile[edfile].topline,editfile[edfile].filename);
	return(NG);
}

chagename(void)
{
	if(fnchage("Edit") == OK) {
		editfile[edfile].writeflag= ON;
		inf();
		return(OK);
	}
	return(NG);
}

fnchage(unsigned char *m)
{
	unsigned char buf1[81],buf2[81],*p;

	if(editfile[edfile].rdonl) {
		msg("\a");
		return(NG);
	}
	buf2[0]= '\0';
	sprintf(buf1,"%s filename = ",m);
	if(inputs(buf2,buf1)== NULL) return(NG);
	p= memgets(buf2);
	if(memerr == NG) return(NG);
	editfile[edfile].filename= p;
	return(OK);
}

save(SCRN *lp,unsigned char *name)
{
	FILE *fp;

	if(editfile[edfile].rdonl) {
		msg("\a");
		return(NG);
	}
	if((fp= fopen(name,"w")) == NULL) {
		msg("System error, why can not open '%s' ??\a",name);
		return(EOF);
	}
	while(lp != NULL) {
		if(lp->fwd != NULL || *lp->buffer)
			fprintf(fp,"%s\n",lp->buffer);
		lp= lp->fwd;
	}
	fclose(fp);
	editfile[edfile].writeflag= OFF;
	inf();
	return(OK);
}

quit(void)
{
	int i,s;

	s = 0;
	for(i = 0;editfile[i].topline != NULL;++i)
		if(editfile[i].writeflag) s = 1;
	if(!s) return(EOF);
	return((ynchk("Quit") == YES)? EOF:NG);
}

filebuf(void)
{
	unsigned short c;

	if((c =inputu("Buffer number = ",edfile+1,NULL)) == 0xffff) return(NG);
	else if(c <= 0 || c >= MAXFILE) return(NG);
	--c;
	if(editfile[c].topline == NULL) return(NG);
	edfile= c;
	line= editfile[edfile].curline;
	curposy= disset(5);
	last= OFF;
}

nxtbuffer(void)
{
	unsigned short c;

	c = edfile + 1;
	if(editfile[c].topline == NULL) c = 0;
	edfile = c;
	line = editfile[edfile].curline;
	curposy = disset(5);
	last= OFF;
}

nxtfile(void)
{
	filewindow(OFF,"File name = ");
}

viewfile(void)
{
	filewindow(ON,"View File = ");
}

#ifdef MSDOS
unsigned char *wildchk(unsigned char nam[])
{
	static unsigned char buf[80];
	unsigned char *p;
	struct stat b;

	if(strchr(nam,'*') != NULL || strchr(nam,'?') != NULL) return(nam);
	if(*nam == '\0') return("*.*");
	p= strchr(nam,'\0'); --p;
	if(*p == '\\' || (isalpha(nam[0]) && nam[1] == ':' && nam[2] == '\0'))
		return(strcat(strcpy(buf,nam),"*.*"));
	else if(stat(nam,&b) != 0) return(NULL);
	if(b.st_mode & S_IFDIR) {
		strcpy(buf,nam);
		p= strchr(buf,'\0');
		if(*(p-1) != '\\') strcat(buf,"\\");
		strcat(buf,"*.*");
		return(buf);
	}
	return(NULL);
}
#endif

fileset(unsigned char *buf,int flg)
{
	int i;
	unsigned char *sp;

#ifndef UNIX
	while((sp= wildchk(buf)) != NULL)
	if((buf= file_name(sp)) == NULL) return(NG);
# ifdef MSDOS
	if((sp=_fullpath(NULL,buf,128)) == NULL) return(NG);
	strlwr(sp);
# endif
#else
	sp = memgets(buf);
#endif
	for(i=0;i < MAXFILE;++i) {
		if(editfile[i].filename != NULL && !stricmp(sp,editfile[i].filename)) {
			edfile= i;
			line= editfile[edfile].curline;
			editfile[edfile].rdonl = flg;
			curposy= disset(5);
			last= OFF; free(sp);
			return(OK);
		}
	}
	editfile[edfile].curline = line;
	for(i=0;i < MAXFILE && editfile[i].topline != NULL;++i) ;
	if(i == MAXFILE) {
		free(sp);
		msg("Full buffer can't read file !\a");
		return(NG);
	}
	edfile= i;
	editfile[edfile].filename= sp;
	editfile[edfile].rdonl = flg;
	readfile();
	return(OK);
}


filewindow(int flag,unsigned char *msg)
{
	unsigned char buf[BUFSIZ], *p;
	int s;

	buf[0]= '\0';
	if(inputs(buf,msg)== NULL) s = NG;
	else {
		if(buf[0] == '\0') p = "*.*";
		else p = buf;
		s = (fileset(p , flag) != NG)?  OK: NG;
		redisplay(CTRL('G'));
	}
	return(s);
}

clearbuffer(int c)
{
 if(editfile[c].topline == NULL) return(NG);
     clearmem(editfile[c].topline,EOF);
      editfile[c].topline= editfile[c].curline= NULL;
 editfile[c].filename[0]= '\0';
  return(OK);
}

killnbuffer(void)
{
unsigned short c;

 if((c =inputu("Kill buffer number = ",edfile+1,NULL)) == 0xffff) return(EOF);
   else if(c <= 0 || c >= MAXFILE) return(EOF);
    --c;
    if(c == edfile) return(NG);
     return(clearbuffer(c));
}

killbuffer(void)
{
unsigned char buf[80];
unsigned char chkn[BUFSIZ];
int i;

        if((i = killnbuffer()) == EOF) return(NG);
      else if(i == OK) return(OK);
    buf[0]= '\0';
   if(inputs(buf,"Kill buffer: ") == NULL) return(NG);
     for(i=0;i < MAXFILE;++i) {
              if(i != edfile && !stricmp(buf, fonly(editfile[i].filename))) {
                 sprintf(chkn, "Kill Buffer %s", editfile[i].filename);
                  if(ynchk(chkn) == YES) return(clearbuffer(i));
          }
       }
       msg("Don't kill buffer [%s] !!", buf);
  return(NG);
}

int ynchk(unsigned char *p)
{
unsigned char buf[8];
unsigned c;

        msg("%s: [y/n]? ",p);
   do {
            c= getkey();
            if(c == eofchar) {
                      msg("Cancel !!\a");
                     return(EOF);
            }
               c= tolower(c);
  } while(c != 'y' && c != 'n');
  vtputc(c);
      return(c == 'y' ? YES:NO);
}

int fileend(void)
{
register i,cp;
SCRN *lp;

    lp= line; i= 0;
 while(lp->fwd != NULL) { 
               lp= lp->fwd; ++i;
       }
       if(i < 10) return(NG);
  line= lp; curposx= 0;
   cp= (lastdisplay - widbase) / 2;
        for(i=0;i < cp;++i) lp= lp->rev;
        distop= lp;
     curposy= setcury(line);
 last= OFF;
      return(OK);
}

int filetop(void)
{
  line= distop= editfile[edfile].topline;
 last= OFF;
      curposy= widbase;
       curposx= 0;
     return(OK);
}
