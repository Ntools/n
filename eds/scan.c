#include "ed.h"
#ifdef MSDOS
# include <search.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#endif

struct ERRFILE {
 struct ERRFILE *fwd;
    char *buffer;
};

char *cc[]= { "Ccompile","CrossAsm","RegidentAsm","" };

#define ERF struct ERRFILE
#define KEYDEFMAX 100

ERF *errmsg= NULL;
ERF *errbuf;

char *ininame;

extern lng;
extern back_flg;
extern struct KEY key[];
extern int beut_flg;

void clrerr(void);
ERF *mgeterr(ERF *);

#ifdef TEXT98
void funcwrite(unsigned short cd,FILE *fp);
unsigned short chkfunckey(char *);
extern unsigned char rev_col;
extern unsigned char nor_col;
#endif


errorfile(void)
{
	extern char *tpnam;
	FILE *fp;
	ERF *ep;
	char buf[90],asmb[100];

	if(errmsg != NULL) clrerr();
	if((fp= fopen(tpnam,"r")) == NULL) return(NG);
	ep= NULL;
	while(fgets(buf,90,fp) != NULL) {
		ep= mgeterr(ep);
		if(errmsg == NULL) errmsg= ep;
		crcut(buf);
		if(lng == ASM)
			sprintf(asmb,"%s %s",editfile[edfile].filename,buf);
		else strcpy(asmb,buf);
		ep->buffer= memgets(asmb);
	}
	if(lng != ASM) {
		if(strstr(errmsg->buffer,editfile[edfile].filename) == NULL) {
			msg("Illegal errfile <illegal filename>\a");
			errbuf= NULL;
			return(NG);
		}
		else errbuf= errmsg->fwd;
	}
	else errbuf= errmsg;
	return(nexterr());
}

nexterr(void)
{
	char *p,*q;
	int num= 0;
	static char erfile[80];

	if(errbuf == NULL) {
		msg("No more error !!");
		errbuf= errmsg->fwd;
		return(NG);
	}
	locate(1,(ldup == 0)? lastdisplay+1: ldup+1);
	rcls();
	vtputs(errbuf->buffer);
	strncpy(erfile,errbuf->buffer,79); erfile[79]= '\0';
	if((p= strchr(erfile,'.')) == NULL) {
		msg("Illegal error file <illegal extention>\a[%s]",erfile);
		return(NG);
	}
	*p= '\0';num= strlen(erfile);
	*p= '.';
	for(++p;isalpha(*p);++p) if(*p == '\0') return(NG);
	*p= '\0';
	p= &errbuf->buffer[num];
	num= 0;
	for( ;!isdigit(*p);++p) if(*p == '\0') return(NG);
	while(isdigit(*p)) {
		num *= 10;
		num += (*p - '0');
		++p;
	}
	p= errbuf->buffer;
	errbuf= errbuf->fwd;
	if(lng == ASM && strchr(p,'+') == NULL) return(OK);
	return(searchnum(num,erfile));
}

searchnum(int num,char *name)
{
	SCRN *lp;
	char *p;

	p = name;
	while((p = strchr(p,'/')) != NULL) *p= '\\';
	if(lng != ASM && strcmp(name,editfile[edfile].filename)) fileset(name,OFF);

	for(lp = editfile[edfile].topline;lp->num != num;lp= lp->fwd) {
		if(lp == NULL) {
			msg("Illegal error file <illegal line number>\a");
			return(NG);
		}
	}
	line= lp;
	curposy= disset((lastdisplay-widbase)/2);
	curposx= 0;
	return(OK);
}

ERF *mgeterr(ERF *ep)
{
	ERF *erp;
	extern memerr;
#ifndef MSDOS
	extern errno;
#endif

	if((erp= (ERF *)malloc(sizeof(ERF))) == NULL) {
		msg("Not enogh memory !![%s]",strerror(errno));
		memerr= NG;
	}
	else {
		if(ep != NULL) ep->fwd= erp;
		memerr= OK;
		erp->buffer= NULL;
		erp->fwd= NULL;
	}
	return(erp);
}

void clrerr(void)
{
	ERF *op,*ep;

	ep = errmsg;
	while(ep != NULL) {
		if(ep->buffer != NULL) free(ep->buffer);
		op = ep; ep= ep->fwd;
		free(op);
	}
	errmsg = NULL;
}

tool(void)
{
	char buf[BUFSIZ],*p;
	unsigned short div2(char *,char **);
	struct KEY kk,*kp;
	size_t cnt;
	FILE *fp;
	int i,sts;

	keyptr = key;
	ininame = "keys.ini";
	if((fp= fopen(ininame,"r")) == NULL) {  /* Curent directory */
		if((p= getenv("KEYS")) != NULL) {
			fp= fopen(p,"r");  /* Env val directory */
		}
	}
	if(fp == NULL) {
#ifndef UNIX
		_searchenv(ininame,"PATH",buf);
#else
		sprintf(buf,"/usr/local/etc/%s", ininame);
#endif
		if((fp= fopen(buf,"r")) == NULL) return(NG);  /* PATH directory */
	}
	if((keyptr = (struct KEY *)calloc(KEYDEFMAX,sizeof(struct KEY))) == NULL)
		abort();
	cnt= 0;
	while(fgets(buf,BUFSIZ,fp) != NULL) {
		crcut(buf);
		if(buf[0] == ';' || buf[0] == '*' || buf[0] == '#') continue;
		if((kk.code = div2(buf,&p)) == 0xffff) {
			keyptr = key; break;
		}
		kk.keyword = buf;
		kp = lfind(&kk, key, &key_count,sizeof(struct KEY),cpkey);
		if(kp == NULL) {
			if(othset(&kk,buf,p) == NG) {
				ndprintf("Keyword not found [%s]!\n",buf);
				if(ynchk("Continue ?") != YES) {
#if (defined TEXT98)
					popscrn();
#elif (defined UNIX)
					ttclose();
#else
					batch("///",NULL);
#endif
					exit(1);
				}
			}
			continue;
		}
		kk.func = kp->func;
		kk.helpmsg = kp->helpmsg;
		if(key_count < KEYDEFMAX) {
			if(cnt) kp = lfind(&kk, keyptr, &cnt,sizeof(struct KEY), codekey);
			else kp= NULL;
			if(kp == NULL) {
				kk.keyword= memgets(buf);
				lsearch(&kk, keyptr, &cnt, sizeof(struct KEY), codekey);
			}
		}
	}
	key_count= cnt;
	fclose(fp);
}

unsigned short div2(char *p,char **pr)
{
	unsigned short cd;
	int sts;

	while(*p > ' ') ++p;
	if(*p != '\0') {
		*p = '\0'; ++p;
		while(*p <= ' ') {
			if(*p == '\0') break;
			else ++p;
		}
	}
	*pr= p;
	if(*p == '\0') return(0xffff);
#ifdef UNIX
	cd = 0;
#else
	if((cd = chkfunckey(p)) != 0) return(cd);
#endif
	while(*p != '\0') {
		if(*p == ' ' || *p == '\t') ++p;
#ifdef UNIX
		else if(strnicmp(p,"DELETE",6) == 0) {
			cd = 0x7f;
			break;
		}
#endif
		else if(strnicmp(p,"meta",4) == 0) {
			p += 4;
			cd |= META;
		}
		else if(strnicmp(p,"extend",6) == 0) {
			p += 6;
			cd |= CTRX;
		}
		else if(strnicmp(p,"esc",3) == 0) {
			p += 3;
			cd |= 27;
		}
		else if(*p == '^') {
			++p; sts = toupper(*p);
			cd |= CTRL(sts); ++p;
		}
		else if(strnicmp(p,"0x",2) == 0) {
			p += 2;
			sscanf(p,"%x",&sts);
			cd |= sts;
			break;
		}
		else if(strncmp(p,"##",2) == 0) break;
		else {
			sts= toupper(*p);
			cd |= sts; ++p;
		}
	}
	return(cd);
}

toolwrite(char *fnam)
{
	int i;
	unsigned short cu,cl;
	FILE *fp;
	char *onf;
	void toolchar(FILE *,unsigned short);

	if((fp= fopen(fnam,"w")) == NULL) return(NG);
	for(i= 0;keyptr[i].keyword != NULL;++i) {
		if(*keyptr[i].keyword == '\0') continue;
		fprintf(fp,"%s",keyptr[i].keyword);
		if(strlen(keyptr[i].keyword) < 8) fprintf(fp,"\t\t");
		else fprintf(fp,"\t");
		cu= keyptr[i].code & 0xff00;
		cl= keyptr[i].code & 0xff;
		if(cu == META) fprintf(fp,"META ");
		else if(cu == CTRX) fprintf(fp,"EXTEND ");
#ifndef UNIX
		else if(cu == FUNC_CODE) {
			funcwrite(keyptr[i].code,fp);
			continue;
		}
#endif
		if(cl <= ' ') fprintf(fp,"^%c\n",(cl+(unsigned)'@'));
		else if(cl < 0x7f) fprintf(fp,"%c\n",cl);
		else if(cl == 0x7f) fprintf(fp,"DEL\n");
		else fprintf(fp,"0x%02x\n",cl);
	}
	for(i= 0;i < 3;++i) {
		if(*copl[i]) fprintf(fp,"%s\t%s\n",cc[i],copl[i]);
		fprintf(fp,"tabset%d\t\t%d\n",i,tabtbl[i]);
	}
	fprintf(fp,"tabset7\t\t%d\n",tabtbl[7]);
	onf= (back_flg)? "ON":"OFF";
	fprintf(fp,"backup\t\t%s\n",onf);
#ifdef TEXT98
	fprintf(fp,"line\t\t%d\n",lastdisplay);
	fprintf(fp,"col\t\t%d\n",displaylen);
	onf= (beut_flg)? "ON":"OFF";
	fprintf(fp,"HighDisplay\t%s\n",onf);
	fprintf(fp,"FColor\t\t%d\n",((nor_col - 1) >> 5));
	fprintf(fp,"BColor\t\t%d\n",((rev_col - 1) >> 5));
#else
	fprintf(fp,"# line\t\t%d\n",lastdisplay);
	fprintf(fp,"# col\t\t%d\n",displaylen);
#endif
	fclose(fp);
}

othset(struct KEY *kp,char *buf,char *p)
{
int s,i;
unsigned col;

      s= NG;
  for(i= 0;i < 3;++i) {
           if(!stricmp(cc[i],buf)) {
                       copl[i]= memgets(p);
                    return(OK);
             }
       }
       if(!strnicmp(buf,"tabset",6)) {
         buf += 6;
               if(*buf < '0' && *buf > '7') return(NG);
                s= *buf - '0';
          if((tabtbl[s]= atoi(p)) == 0) return(NG);
               return(OK);
     }
       else if(!stricmp(buf,"backup")) {
               if(!strnicmp(p,"on",2) && *(p + 2) <= ' ')
                      back_flg = ON;
          else if(!strnicmp(p,"off",3) && *(p + 3) <= ' ')
                        back_flg = OFF;
         else return(NG);
                return(OK);
     }
       else if(!stricmp(buf,"line")) {
         if((lastdisplay= atoi(p)) < 20) {
                       fprintf(stderr,"Like wow, man. No lines or columns!\n");
                        exit(1);
                }
               return(OK);
     }
       else if(!stricmp(buf,"col")) {
          if((displaylen = atoi(p)) < 40) {
                       fprintf(stderr,"Like wow, man. No columns!\n");
                 exit(1);
                }
               return(OK);
     }
       else if(!stricmp(buf,"highdisplay")) {
          if(!strnicmp(p,"on",2) && *(p + 2) <= ' ')
                      beut_flg = ON;
          else if(!strnicmp(p,"off",3) && *(p + 3) <= ' ')
                        beut_flg = OFF;
         else return(NG);
                return(OK);
     }
#ifdef TEXT98
  else if(!stricmp(buf,"bcolor")) {
               col = atoi(p);
          if(col > 7 || col == 0) return(NG);
             rev_col = (col << 5) + 5;
               return(OK);
     }
       else if(!stricmp(buf,"fcolor")) {
               col = atoi(p);
          if(col > 7 || col == 0) return(NG);
             nor_col = (col << 5) + 1;
               return(OK);
     }
#endif
 return(NG);
}
