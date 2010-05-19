# See COPYING file for copyright, license and warranty details.

VERSION = 0.2

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# includes and libs
INCS = -I. -I/usr/include
LIBS = -L/usr/lib -lc

CURLINC = $(shell pkg-config --cflags libcurl)
CURLLIB = $(shell pkg-config --libs libcurl)
XMLINC = $(shell pkg-config --cflags libxml-2.0)
XMLLIB = $(shell pkg-config --libs libxml-2.0)

# flags
#CFLAGS = -pedantic -Wall -Werror ${INCS} ${CURLINC} ${XMLINC} -DVERSION=\"${VERSION}\"
CFLAGS = -std=c99 -Wall -Werror ${INCS} ${CURLINC} ${XMLINC} -DVERSION=\"${VERSION}\"
LDFLAGS = ${LIBS} ${CURLLIB} ${XMLLIB}

# compiler and linker
CC = cc
