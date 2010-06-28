# See COPYING file for copyright, license and warranty details.

VERSION = 0.3

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# includes and libs
CURLINC = $(shell pkg-config --cflags libcurl)
CURLLIB = $(shell pkg-config --libs libcurl)
XMLINC = $(shell pkg-config --cflags libxml-2.0)
XMLLIB = $(shell pkg-config --libs libxml-2.0)

INCS = -I. -I/usr/include ${CURLINC} ${XMLINC}
LIBS = -L/usr/lib -lc ${CURLLIB} ${XMLLIB}

# flags
CFLAGS = -std=c99 -Wall -Werror ${INCS} -DVERSION=\"${VERSION}\"
LDFLAGS = ${LIBS}

# compiler and linker
CC = cc
