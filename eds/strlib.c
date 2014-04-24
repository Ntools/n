/*
   stricmp (code)
   strnicmp (code)
*/

#include	<stdio.h>
#include	<ctype.h>
#ifdef LINUX
# include	<slcurses.h>
#define KEY_DC		SL_KEY_DELETE		/* Delete character */
#define KEY_IC		SL_KEY_IC			/* Insert char or enter insert mode */
#define KEY_EOL		0517				/* Clear to end of line */
#else
# include	<curses.h>
#endif

#define   NG    0

typedef unsigned char PTR;

stricmp(PTR *p1, PTR *p2)
{
	unsigned int c1, c2;

	do {
		c2 = (*p2 >= 'a' && *p2 <= 'z')? (*p2 & 0x5f): *p2;
		c1 = (*p1 >= 'a' && *p1 <= 'z')? (*p1 & 0x5f): *p1;
		if(c1 - c2) return(c1 - c2);
		++p1; ++p2;
	} while (c1);
	return(c1 - c2);
}

strnicmp(PTR *p1, PTR *p2, int n)
{
	unsigned int c1, c2;

	while(n--) {
		c2 = (*p2 >= 'a' && *p2 <= 'z')? (*p2 & 0x5f): *p2;
		c1 = (*p1 >= 'a' && *p1 <= 'z')? (*p1 & 0x5f): *p1;
		if(c1 - c2) return(c1 - c2);
		if(c1 == '\0' || c2 == '\0') break;
		++p1; ++p2;
	}
	return(c1 - c2);
}

#if 0
void *l_search(const void *key, const void *base, unsigned int *num,
             unsigned int width, int (*compare)())
{
	unsigned int cnt;
	unsigned char *p;

	cnt = *num;
	p = base;
	while(cnt--) {
		if((*compare)(p, key) == 0) return((void *)p);
		p = &p[width];
	}
	return(NULL);
}

void *search_apd(const void *key, void *base, unsigned int *num,
	     unsigned int width, int (*compare)())
{
	unsigned int cnt;
	unsigned char *p;

	cnt = *num;
	p = base;
	while(cnt--) {
		if((*compare)(p, key) == 0) return((void *)p);
		p = &p[width];
	}
	for(cnt = 0;cnt < width;++cnt) p[cnt] = *((unsigned char *)key + cnt);
	*num += 1;
	return((void *)p);
}
#endif
kpadsearch(unsigned int k, int *s)
{
	switch(k) {
	  case KEY_IC: *s = inson(); break;
      case KEY_UP: *s = upl(); break;
      case KEY_DOWN: *s = dwnl(); break;
      case KEY_RIGHT: *s = rightl(); break;
      case KEY_LEFT: *s = leftl(); break;
      case KEY_HOME: *s = leftgo(); break;
	  case KEY_DC: *s = delchar(); break;
      case KEY_EOL: *s = rightgo(); break;
      case KEY_NPAGE: *s = pageup(); break;
      case KEY_PPAGE: *s = pagedwn(); break;
      case KEY_BACKSPACE : *s = bkspc(); break;
      default: *s = NG; return(NG);
	}
	return(1);
}
