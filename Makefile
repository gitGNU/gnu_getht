# See COPYING file for copyright, license and warranty details.

include config.mk

TARGET = getht
SRC = $(shell find . -name '*.c')
OBJ = $(SRC:.c=.o)
MAN = $(TARGET:=.1)
DOC = README

all: $(TARGET)

$(TARGET:=.o): $(SRC)
	cc -c $(SRC) $(CFLAGS)

$(TARGET): $(OBJ)
	cc -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -f -- $(TARGET) $(OBJ)

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(TARGET) $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp -f $(MAN) $(DESTDIR)$(MANPREFIX)/man1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	rm -f $(DESTDIR)$(MANPREFIX)/man1/$(MAN)

dist: clean
	mkdir -p getht-$(VERSION)
	cp -R $(SRC) $(DOC) getht-$(VERSION)
	sed "s/VERSION/$(VERSION)/g" < $(MAN) > getht-$(VERSION)/$(MAN)
	tar -c getht-$(VERSION) > getht-$(VERSION).tar
	bzip2 < getht-$(VERSION).tar > getht-$(VERSION).tar.bz2
	rm -rf getht-$(VERSION).tar getht-$(VERSION)
	gpg -b < getht-$(VERSION).tar.bz2 > getht-$(VERSION).tar.bz2.sig
	sha1sum < getht-$(VERSION).tar.bz2 > getht-$(VERSION).tar.bz2.sha1
