PREFIX = ${HOME}/.local
LIBS = -lmpdclient
CFLAGS = --std=c99 -pedantic -Wall -Wextra -D_POSIX_C_SOURCE=200809L
LDFLAGS = ${LIBS}
CC = cc

SRC = sb-track.c
OBJ = ${SRC:.c=.o}

all: sb-track

sb-track: sb-track.c
	${CC} -o $@ ${SRC} ${LDFLAGS} ${CFLAGS}

install: all
	mkdir -p ${PREFIX}/bin
	chmod 755 sb-track
	cp -f sb-track ${PREFIX}/bin

clean:
	rm -f ${OBJ} sb-track
