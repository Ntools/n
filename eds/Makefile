# -------------------------------------------------
#
#     Makefile for Linux
#
# -------------------------------------------------
CC = gcc
#CFLAGS = -O -DSLANG -DUNIX -DLINUX -I/usr/include/slang
CFLAGS = -g -DSLANG -DUNIX -DLINUX -I/usr/include/slang
# -DDBG
LFLAGS = -g
FILES= main.o xout.o display.o mem.o funckey.o \
	function.o ismachin.o jmptbl.o key.o scan.o search.o \
	strlib.o update.o sysV.o

NEFILES= main vram display mem funckey function ismachin jmptbl \
	key scan search strlib update sysV.o

#LIBS = -lslang-utf8
LIBS = -lslang

EXECDIR = /usr/bin

OBJS= n

all:	$(OBJS)

install:	$(OBJS)
	install -c -m 755 $(OBJS) $(EXECDIR)
	install -c -m 644 keys.ini /usr/local/etc

src:
	lha e srced.lzh

obj:
	lha e pacobj.lzh

clean:
	rm -f core *.o *.bak #*#

del:
	lha u srced.lzh *.h *.c Makefile
	rm *.h
	rm *.c
	rm *.o
	rm *.bak

$(OBJS): $(FILES)
	$(CC) -o $(OBJS) $(LFLAGS) $(FILES) $(LIBS)

main.o:	main.c ed.h Makefile
display.o:	display.c ed.h
funckey.o:	funckey.c ed.h
function.o:	function.c ed.h
ismachin.o:	ismachin.c
jmptbl.o:	jmptbl.c ed.h
key.o:		key.c ed.h
mem.o:		mem.c ed.h
update.o:	update.c ed.h
scan.o:		scan.c ed.h
search.o:	search.c ed.h
strlib.o:	strlib.c
sysV.o:		sysV.c 
xout.o:		xout.c
