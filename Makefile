# stask - simple task manager
# See LICENSE file for copyright and license details.

include config.mk

SRC = stask.c data.c gui.c
OBJ = ${SRC:.c=.o}

all: stask

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: stask.h config.h

config.h:
	cp config.def.h config.h

stask: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f stask ${OBJ}

install: stask
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f stask ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/stask

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/stask

.PHONY: all clean install uninstall
