PREFIX = /usr/local
CF_SHARE = $(PREFIX)/share/confront
CFLAGS = -DCF_SHARE=\"$(CF_SHARE)\"

ifdef DEBUG
CFLAGS += -Og -g -gdwarf-4 -fkeep-inline-functions -fPIC
else
CFLAGS += -O2 -fPIC
endif

OBJS=confront.o
LDLIBS=-lSDL2 -lSDL2_image -lSDL2_mixer

all: confront

confront: $(OBJS)

test: all
	./confront --joy=0

install: all
	install -s confront $(PREFIX)/bin/
	mkdir -p $(CF_SHARE)/
	install -m 644 confront.conf $(CF_SHARE)/
	mkdir -p $(CF_SHARE)/images/
	install -m 644 images/* $(CF_SHARE)/images/
	mkdir -p $(CF_SHARE)/sounds/
	install -m 644 sounds/* $(CF_SHARE)/sounds/
	mkdir -p $(PREFIX)/share/pixmaps/
	install -m 644 confront.png $(PREFIX)/share/pixmaps/
	mkdir -p $(PREFIX)/share/applications/
	install -m 644 confront.desktop $(PREFIX)/share/applications/

clean:
	rm -f confront $(OBJS)

