
#include        <graph.h>
#include       <stdio.h>
#include       <string.h>
#include      <ctype.h>

#define        SET_DEF         0
#define        NO_COLOR        (-1)

long col[] = {
      _98BLACK,       _98BLUE,        _98GREEN,       _98CYAN,
        _98RED,         _98MAGENTA,     _98YELLOW,      _98WHITE,
       _98GRAY,        _98DARKBLUE,    _98DARKGREEN,   _98DARKCYAN,
    _98DARKRED,     _98DARKMAGENTA, _98DARKYELLOW,  _98DARKWHITE,
   NO_COLOR
};

char *c_name[] = {
    "Black",       "Blue",        "Green",       "Cyan",
    "red",         "Magenta",     "Yellow",      "White",
   "Gray",        "Dark Blue",    "Dark Green",   "Dark Cyan",
     "Dark Red",     "Dark Magenta", "Dark Yellow",  "Dark White",
};

void printcolor(long c)
{
int i;

   for(i = 0;i < 16;++i) {
         if(c == col[i]) {
                       printf("Back Grand Color = [%s],\t\tColor Code = [%d].\n", c_name[i], i);
                       return;
         }
       }
       printf("Unknown Back Grand Color [%lu] !!\n", c);
}

void help(char *p, int f)
{
int i;
char buf[80], *sp;

    if((sp = strrchr(p, '\\')) == NULL) sp = p;
     else ++sp;
      strcpy(buf, sp);
        if((sp = strchr(buf,'.')) != NULL) *sp = '\0';
  strlwr(buf); buf[0] = toupper(buf[0]);
  fprintf(stderr,"Syntax: %s [<opts>] [<opts>]\n",buf );
  fprintf(stderr,"Function: Change Back Grand Color.\n");
 fprintf(stderr,"Option:  -C=<Code>       Set Color <Code>\n");
  fprintf(stderr,"         -R=<Hex Data>   Set Red lebel to <data>\n");
   fprintf(stderr,"         -G=<Hex Data>   Set Green lebel to <data>\n");
 fprintf(stderr,"         -B=<Hex Data>   Set Blue lebel to <data>\n");
  fprintf(stderr,"         -D              Default Video mode\n");
        fprintf(stderr,"         -?              Help menu\n");
 if(!f) exit(128);
       for(i = 0;i < 16;++i)
           fprintf(stderr,
                 "Code %2d,\tBack Grand Color is [%s].\n", i, c_name[i]);
        exit(3);
}

long setcol(char *p, unsigned sht)
{
long cl;
unsigned l;

 if(*p == '=') ++p;
      sscanf(p, "%x", &l);
    cl = l;
 cl <<= sht;
     return(cl);
}

main(int argc, char *argv[])
{
int i, c;
char *p;
long b_color = 0;

     if(argc == 1) {
         printcolor(_getbkcolor());
              exit(0);
        }
       for(i = 1;i < argc;++i) {
               p = argv[i];
            if(*p++ == '-') {
                       switch(toupper(*p)) {
                           case 'R': ++p; b_color += setcol(p, 0); break;
                          case 'G': ++p; b_color += setcol(p, 8); break;
                          case 'B': ++p; b_color += setcol(p, 16); break;
                         case 'C':               /* back grand color */
                                  ++p;
                                    if(*p == '=') ++p;
                                      c = atoi(p);
                                    if(c > 15 || c < 0) {
                                           fprintf(stderr, "Unknown code %s", p);
                                          exit(255);
                                      }
                                       b_color = col[c];
                                       goto setc;
                              case 'D':               /* Default setting */
                                   _setbkcolor(0);
                                 _setvideomoderows(_DEFAULTMODE, 24);
                                    exit(0);
                                default :
                                       fprintf(stderr, "Unknown Option %s !!\n", argv[i]);
                             case '?':               /* Default setting */
                                   ++p;
                                    help(argv[0], (*p != '\0')? 1:0);
                                       break;
                  }
               }
       }
setc:
  if(_getbkcolor() != 0) {
                _setbkcolor(0);
         _setvideomode(_DEFAULTMODE);
    }
       _setvideomode(_98RESS16COLOR);
  _setbkcolor(b_color);
   return(0);
}
