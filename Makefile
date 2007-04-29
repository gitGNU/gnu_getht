PREFIX=/usr
DOCS=AUTHOR COPYING CREDITS ChangeLog INSTALL README TODO
CFLAGS+=-g `curl-config --cflags` `xml2-config --cflags`
LDFLAGS+=`curl-config --libs` `xml2-config --libs`

all: getht

getht: download.o config.o issuemem.o tocxml.o mediarev.o mediaxml.o xml.o

tocxml.o mediaxml.o: xml.o

static:
	CFLAGS="$(CFLAGS) -s -static"
	@echo "Not implemented yet, sorry."

clean:
	rm -rf *.o

install:
	cp getht $(PREFIX)/bin
	mkdir -p $(PREFIX)/doc/getht
	cp $(DOCS) $(PREFIX)/doc/getht

uninstall:
	rm $(PREFIX)/bin/getht
	rm -r $(PREFIX)/doc/getht

help:
	@echo -e "Make targets:"
	@echo -e "make (all)\tBuilds GetHT with default settings"
	@echo -e "make install\tInstalls GetHT"
	@echo -e "make uninstall\tRemoves GetHT"
	@echo -e "make clean\tRemoves all object files"
	@echo -e "make static\tBuilds a statically linked GetHT binary"
	@echo -e "make help\tPrint this help message"

.PHONY: all clean install uninstall help
