#include "ed.h"

extern int refr_flg;
extern int crp;
unsigned char searchbuf[80];
unsigned char *find;
unsigned intval = 1;

void set_undo(void)
{
register int i;

        for(i= (UNDOLEVEL-1);i > 0;--i) ;
}

int undoexec(void)
{
   return(NG);
}

str_cmpn(unsigned char *s,unsigned char *t, size_t n)
{
      for(;n-- && *t;++t,++s) {
               if(*t == '?' || *s == *t) ;
             else return(*s - *t);
   }
       return(0);
}

char *strsearch(unsigned char *s, unsigned char *t)
{
size_t   n;

      n = strlen(t);
  for (; *s; s++)
         if (str_cmpn(s, t, n) == 0)
                     return (s);
     return (NULL);
}

int bufset(unsigned char buf[])
{
int s;

   do {
            if((s= charin(buf,"Search strings= ")) == EOF) return(NG);
      } while(s != '\r') ;
    strcpy(searchbuf,buf);
  return((searchbuf[0] != '\0')? OK:NG);
}

int argin(void)
{
unsigned char buf[80];

   buf[0]= '\0'; strcpy(buf,searchbuf);
    return(bufset(buf));
}

int argset(void)
{
  if(argin() == NG) return(NG);
   if(search() == NG) rsearch();
   return(NG);
}

int setpara(void)
{
unsigned char *p;
unsigned char buf[80];
int i;

     p = &line->buffer[curposx];
     while(*p == ' ' || *p == '\t') ++p;
     for(i = 0;i < 80;++i) {
         if(!chkword(*p)) break;
         buf[i] = *p++ ;
 }
       buf[i] = '\0';
  if(bufset(buf) == NG) return(NG);
       if(search() == NG) {
            if(rsearch() != NG) rsearch();
  }
       else search();
  return(NG);
}

int replace(void)
{
extern posxflag;
int l,cnt,s;
unsigned char buf[SCRN_BUF];
static unsigned char serchst[80];
static unsigned char replcst[80];

        strcpy(buf,serchst);
    if(inputs(buf,"Search strings: ")== NULL) return(NG);
   if(buf[0]) strcpy(serchst,buf);
 strcpy(buf,replcst);
    if(inputs(buf,"Replace strings: ")== NULL) return(NG);
  if(buf[0]) strcpy(replcst,buf);
 l= strlen(serchst); cnt= 0;
     while(searchexe(ON,serchst)) {
          curposx -= l;
           display();
              posxflag = OFF;
         locate(poschk(),curposy + 1); htprintf("%s",serchst);
           curposx += l;
           if((s= ynchk("Replace string execute")) == NO) continue;
                else if(s == EOF) break;
                strcpy(buf,line->buffer);
               buf[(find - line->buffer)]= '\0';
               strcat(buf,replcst);
            strcat(buf,find + l);
           s= strlen(replcst) - l;
         if(update(buf,s) == NG) break;
          ++cnt;
  }
       msg("%4d occurrences replaced",cnt);
    return(NG);
}

int search(void)
{
   if(searchbuf[0] == '\0' && argin() == NG) return(NG);
   msg("+Search [%s]",searchbuf);
  return(searchexe(ON,searchbuf));
}

int rsearch(void)
{
     if(searchbuf[0] == '\0' && argin() == NG) return(NG);
   msg("-Search [%s]",searchbuf);
  return(searchexe(OFF,searchbuf));
}

int searchexe(int flg,unsigned char *sbuf)
{
unsigned char *p;
SCRN *lp;
int s;

   s= NG;
  if(!sbuf[0]) return(NG);

        lp= line; p= lp->buffer+curposx;
        while(1) {
              if((p= strsearch(p,sbuf)) != NULL) {
                    find= p;
                        line= lp; s= OK;
                        curposy= disset(5);
                     curposx= p - lp->buffer;
                        curposx += strlen(sbuf);
                        crp = OFF;
                      break;
          }
               if(flg) lp= lp->fwd;
            else lp= lp->rev;
               if(lp == NULL) {
                        msg("Not found ");
                      break;
          }
               p= lp->buffer;
  }
       return(s);
}

setdistp(void)
{
      distop= line;
   curposy= widbase;
       refr_flg= last= OFF;
}

linejmp(void)
{
      SCRN *lp;
       unsigned l;

     if((l = inputu("Line Number = ", line->num , NULL)) == 0xffff || line->num == l) return(NG);
    for(lp = editfile[edfile].topline;lp->num != l;lp = lp->fwd)
            if(lp == NULL) return(NG);
      line = lp;
      curposx = 0;
    curposy = disset(5);
    refr_flg = OFF;
}

int disset(register int i)
{
SCRN *lp;
register int f;

      lp= line;
       for(f= 0;i--;++f) {
             if(lp->rev == NULL) break;
              lp= lp->rev;
    }
       distop= lp;
     last= OFF;
      return(f+widbase);
}

inputval()
{
unsigned c;

       if((c = inputu("Initial Value = ", intval , NULL)) == 0xffff) return(NG);
       intval = c;
     return(OK);
}

digit()
{
unsigned c, v;
int s;

        v = intval; s = NG;
     for(c = 10000;c > 0;c /= 10) {
          if(s != NG || v >= c) 
                  if((s = charactor(v / c + '0')) == NG) break;
           v %= c;
 }
       ++intval;
       return(s);
}
