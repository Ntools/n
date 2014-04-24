/*
-------------------------- Set File name <setfname.c> ------------------------
*/
#ifdef _MSC_VER
#include <search.h>
#include <direct.h>
#define  ff_name         name
#define     ff_attrib       attrib
#endif

#ifdef __TURBOC__
#include   <dir.h>
#define  _dos_findfirst(p,a,b)   findfirst((p),(b),(a))
#define   _dos_findnext(b)                findnext((b))
#define    find_t  ffblk
#endif

#include "ed.h"

#ifndef DEBUG
#define printf   ndprintf
#define getch    getkey
#endif

#define UP          1
#define DOWN   2
#define RIGHT  3
#define LEFT   4
#define EOFCHAR 5
#define BS            6

#define ESC                    0x1b
#define TOPFRAME    5
#define FRAMECNT       (displaylen/16)

#ifdef _MSC_VER
#define _A_FILE   _A_RDONLY|_A_SUBDIR|_A_ARCH
#endif
#ifdef __TURBOC__
#define _A_FILE       FA_RDONLY|FA_DIREC|FA_ARCH
#define       _A_SUBDIR       FA_DIREC
/*
struct ffblk {
  char      ff_reserved[21];
  char      ff_attrib;
  unsigned  ff_ftime;
  unsigned  ff_fdate;
  long      ff_fsize;
  char      ff_name[13];
};
*/
#endif

#define      dname(nam) (strcpy(nam,donly(nam)))

struct ndir {
        char *name;
     char attrib;
};

typedef   struct ndir DIRN;

DIRN *dirbuf;
static int si= 0xff;

unsigned short eofchar= ESC;
size_t filemax;

int setcode(unsigned short c)
{
struct KEY *kp,kk;
int codekey( const void *, const void *);

  if(c == eofchar) c= EOFCHAR;
    else {
          if(c == METACHAR)
                       c = META | getch();
             kk.code= c;
             kp = lfind(&kk,keyptr,&key_count,sizeof(struct KEY),codekey);
           if(kp != NULL) {
                        if(!stricmp(kp->keyword,"up")) c= UP;
                   else if(!stricmp(kp->keyword,"down")) c= DOWN;
                  else if(!stricmp(kp->keyword,DEFSTR5)) c= LEFT;
                 else if(!stricmp(kp->keyword,DEFSTR6)) c= RIGHT;
                        else if(!stricmp(kp->keyword,DEFSTR3)) c= BS;
           }
               else if(c < ' ') c= 0;
  }
       return(c);
}

last_en(char *pth)
{
char *p;

  if((p = strchr(pth,'\0')) == NULL) return(OFF);
 if(p == pth) return(OFF);
       --p;
    if(*p == '\\') return(ON);
      return(OFF);
}

void dirfree(int cnt)
{
register i;
  for(i=0;i < cnt;++i) free(dirbuf[i].name);
      free(dirbuf);
}

char *donly(char path[])
{
char *p;
static char buf[80];
register i;

  if((p= strrchr(path,'\\')) != NULL) {
           if(*(p - 1) == ':') ++p;
        }
       else if(path[1] == ':') p= &path[2];
    else p= path;
   for(i= 0;i < (p - path); ++i) buf[i]= path[i];
  buf[i]= '\0';
   return(buf);
}

char *fonly(const char *path)
{
char *p;

     if((p= strrchr(path,'\\')) != NULL) ++p;
        else if(path[1] == ':') p= (char *)&path[2];
    else p= (char *)path;
   return(p);
}

char *extonly(char path[])
{
char *ps, *p;

     ps = fonly(path);
       if((p = strchr(ps, '.')) != NULL && p[1] != '\0' && p[1] != '.') ps = p + 1;
    else ps = "";
   return(ps);
}

int chkext(char *nam)
{
char *p;

      if(strcmp(nam, ".") == 0) return 0;
     if(strcmp(nam, "..") == 0) return 0;
    p = extonly(nam);
       if(stricmp(p, "com") == 0) return 0;
    if(stricmp(p, "dll") == 0) return 0;
    if(stricmp(p, "obj") == 0) return 0;
    if(stricmp(p, "exe") == 0) return 0;
    if(stricmp(p, "rel") == 0) return 0;
    if(stricmp(p, "lzh") == 0) return 0;
    if(stricmp(p, "zip") == 0) return 0;
    if(stricmp(p, "arc") == 0) return 0;
    else return(ON);
}

int extcmp(const void *p1, const void *p2)
{
int c;

      c = (int)(((DIRN *)p2)->attrib & _A_SUBDIR) - (int)(((DIRN *)p1)->attrib & _A_SUBDIR);
  if(c) return( (c > 0)? 0x7fff: -0x8000);
        c = strcmp(extonly(((DIRN *)p1)->name),extonly(((DIRN *)p2)->name));
    if(c) return( (c > 0)? 0x3fff: -0x4000);
        return(strcmp(fonly(((DIRN *)p1)->name),fonly(((DIRN *)p2)->name)));
}

DIRN *dir(char *buf,int *i)
{
struct find_t bf;
char path[128], fpb[128], *p, *sp;

    *i = 0;
 sp = donly(buf);
        strcpy(path,sp);
        p = strchr(path,'\0');
  if(p == path || *(p - 1) != '\\') {
             if(p == path || *(p -1) == ':') strcat(path,"..");
              else strcat(path, "\\..");
              if(_fullpath(fpb,path,120) == NULL)
                     strcpy(fpb, path);
              if(last_en(fpb)) strcat(fpb, "*.*");
            else strcat(fpb, "\\*.*");
              if(_dos_findfirst(fpb,_A_FILE,&bf) == 0) {
                      dirbuf[0].name= strdup(path);
                   dirbuf[0].attrib = _A_SUBDIR;
                   *i += 1;
                }
       }
       strcpy(path,buf);
       p= fonly(path);
 if(_dos_findfirst(buf,_A_FILE,&bf) != 0) return(dirbuf);
        while(*i < filemax ) {
          if(chkext(bf.ff_name)) {
                        *p= '\0';
                       strcat(path,bf.ff_name);
                        if((sp=_fullpath(NULL,path,128)) == NULL) sp = strdup("<<NULL>>");
                      else dirbuf[*i].name= strlwr(sp);
                       dirbuf[*i].attrib = bf.ff_attrib;
                       *i += 1;
                }
               if(_dos_findnext(&bf) != 0) break;
      }
       qsort(dirbuf,(size_t)*i,sizeof(DIRN),extcmp);
   return(dirbuf);
}

void prtfnam(int s,int flg)
{
int x,y;
char *p,*fmt;
int (*fnc)();

/*               1234567890123456 */
      if(flg) msg("Edit Filename = ");
        x= (s % FRAMECNT) * (displaylen/FRAMECNT) + 2;
  y= (s / FRAMECNT) + TOPFRAME + 1;
       locate(x,y); p= fonly(dirbuf[s].name);
  y= strlen(p);
   if(dirbuf[s].attrib & _A_SUBDIR) {
              x= (displaylen / FRAMECNT - 4)-y;
               y= ON;
  } else {
                x= (displaylen / FRAMECNT - 2)-y;
               y= OFF;
 }
       sppcnt(x - 1,OFF);
      fnc= (flg)? htprintf: ndprintf;
  fmt= (y)? "[%s]":"%s";
  (*fnc)(fmt,p);
  if(flg) {
               locate(17,lastdisplay+1);
               htprintf("%s",p);
               vtputs(" ");
    }
}

int dispfl(int max,int n)
{
register i,j;
char *p;

       if(si == 0xff) {
                cls();
          locate(1,TOPFRAME-2);
           printf("Directory %s  %d File(s).",donly(dirbuf[max-1].name),max);
              locate(1,TOPFRAME);
             sppcnt(displaylen,ON);
          j= (max / FRAMECNT) + ((max % FRAMECNT)? 1:0);
          for(i= 0;i < j;++i) {
                   locate(1,i+TOPFRAME+1);
                 htprintf(" ");
                  locate(displaylen,i+TOPFRAME+1);
                        htprintf(" ");
          }
               sppcnt(displaylen,ON);
          for(i= 0;i < max;++i) prtfnam(i,(i == n));
      }
       else if(si != i) {
              prtfnam(si,0);
          prtfnam(n,1);
   }
       si = n;
 i= getch();
     return(i);
}

char *dirname(char *nam,char *dnam)
{
char buf[128];

   strcpy(buf,fonly(nam));
 strcpy(nam,dnam);
       strcat(nam,"\\");
       strcat(nam,buf);
        return(nam);
}


char *file_name(char *nam)
{
        extern arg_flg;
 register i,j;
   int cnt, lstd;
  char *p,buf[SCRN_LEN+1];
        static char name[128];
  char *wildchk(char *);

  filemax = (lastdisplay - TOPFRAME -1) * FRAMECNT;
       if((dirbuf= calloc(filemax,sizeof(DIRN))) == NULL) exit(6);
     lstd = lastdisplay; lastdisplay= TOPFRAME - 2;
  curoff();
       p= strcpy(name,nam);
    while(1) {
toploop:
              si= 0xff;
               dir(name,&cnt);
         if(!cnt) {
                      name[0] = '\0';
                 curon();
                        if(inputs(name,"Edit Filename = ") != NULL) {
                           if((p = wildchk(buf)) != NULL) {
                                        strcpy(name,p);
                                 curoff();
                               }
                               else {
                                  p = name;
                                       goto lpout;
                             }
                       }
                       continue;
               }
               for(j = i = 0;i < cnt;++i) {
                    if(!(dirbuf[i].attrib & _A_SUBDIR)) {
                           ++j;
                            p = strcpy(name, dirbuf[i].name);
                       }
               }
               if(j == 1 && arg_flg) goto lpout;
               for(i= 0;(j= dispfl(cnt,i)) != '\r';) {
#ifndef DEBUG
                    j= setcode(j);
#endif
                    switch(j) {
                             case UP :
                                       if(i >= FRAMECNT) i -= FRAMECNT;
                                        break;
                          case DOWN :
                                     i += FRAMECNT;
                                  if(i >= cnt) i -= FRAMECNT;
                                     break;
                          case RIGHT :
                                    ++i;
                                    if(i >= cnt) i= 0;
                                      break;
                          case LEFT :
                                     if (!i) i= cnt;
                                 --i;
                                    break;
                          default:
                                        if(j <= ' ' || j == 0x7f || j >= 0xff) break;
                                   ungetkey();
                             case BS :
                                       buf[0]= '\0'; prtfnam(si,0);
                                    curon();
                                        if(inputs(buf,"Edit Filename = ") != NULL) {
                                            if((p = wildchk(buf)) != NULL) {
                                                        strcpy(name,p);
                                                 curoff();
                                                       goto toploop;
                                           }
                                               else {
                                                  p = strchr(buf,'\0'); --p;
                                                      if(*p == '\\') *p= '\0';
                                                        sprintf(name,"%s\\%s",(cnt)? donly(dirbuf[cnt-1].name):".",buf);
                                                        p = name;
                                                       goto lpout;
                                             }
                                       }
                                       curoff();
                                       msg(""); prtfnam(si,1);
                                 break;
                          case EOFCHAR :
                                  p= NULL;
                                        goto lpout;
                     }
               }
               if((dirbuf[i].attrib & _A_SUBDIR) != 0) dirname(name,dirbuf[i].name);
           else break;
     }
       p = strcpy(name,dirbuf[i].name);
lpout:
  curon();
        dirfree(cnt);
   lastdisplay= lstd;
      return(p);
}

#ifdef DEBUG
void main(int argc,char **argv)
{
int i;
char *p;

   for(i = 1;i < argc;++i) {
               p= file_name(argv[i]);
  }
       cls();
  if(p != NULL) printf("%s",p);
}
#endif

#if (defined _MSC_VER) || (defined _LSIC)

#else

#ifdef __TURBOC__
cdd(char *p)
{
int d,cd;

       d= getdisk();
   if(*(p + 1) == ':') {
           cd= toupper(*p) - 'A';
          if(cd != d)
                     setdisk(cd);
    }
       d= chdir(p);
    return(d);
}
#else
cdd(char *p)
{
   return(chdir(p));
}
#endif

char *_fullpath(char rbuf[], char *pname, unsigned ml)
{
char *p;
char cbuf[BUFSIZ];

       if(rbuf == NULL) rbuf = malloc(ml);
     if(getcwd(cbuf,BUFSIZ) == NULL) return(NULL);
   p = donly(pname);
       if(*p != '\0') cdd(p);
  if(getcwd(rbuf,ml) == NULL) return(NULL);
       p = strchr(rbuf, '\0');
 if(p != rbuf && *(p - 1) != '\\') strcat(rbuf, "\\");
   p = fonly(pname);
       strcat(rbuf, p);
        cdd(cbuf);
      return(rbuf);
}
#endif
