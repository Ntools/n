#include <termio.h>
#include <stdlib.h>
#include <stdio.h>

#define	CTRL(c)		((c)-'@')

static struct termio	oldtermio;	/* original tty mode */
static struct termio	newtermio;	/* cbreak/noecho tty mode */
unsigned ERASEKEY;

void backset()
{
	/* change the terminal mode back the way it was */
	ioctl(0, TCSETAW, &oldtermio);
}

void keyset()
{
	/* get curent terminal mode to cbreak/noecho */

	ioctl(0, TCGETA, &oldtermio);

	/* change the terminal mode to cbreak/noecho */

/*	ospeed = (oldtermio.c_cflag & CBAUD); */
	ERASEKEY = oldtermio.c_cc[VERASE];
	newtermio = oldtermio;
	newtermio.c_iflag &= (IXANY|ISTRIP|IGNBRK);
	newtermio.c_oflag &= ~OPOST;
	newtermio.c_lflag = ISIG;
	newtermio.c_cc[VINTR] = -1; /* always use ^C for interrupts */
	newtermio.c_cc[VMIN] = 1;
	newtermio.c_cc[VTIME] = 0;
#ifdef VSWTCH
	newtermio.c_cc[VSWTCH] = 0;
#endif
	ioctl(0, TCSETA, &newtermio);
}

main(ac,av)
int ac;
char *av[];
{
	char buf[BUFSIZ];
	int	i;

	buf[0] = '\0';
	keyset();
	strcpy(buf, "nd");
	for(i = 1;i < ac;++i) {
		strcat(buf, " ");
		strcat(buf, av[i]);
	}
	printf("%s\r\n", buf);
	i = system(buf);
	backset();
	exit(i);
}



