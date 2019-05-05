CC = g++ 

EXECUTABLE = F4HTBpna

CFLAGS += -Wall -Wextra -g -O0
LDLIBS += -lm -lasound -lfftw3 -lpthread 

INSTALL=install
INSTALL_PROGRAM=$(INSTALL)
INSTALL_DATA=$(INSTALL) -m 644

prefix=/usr/local
exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin
datarootdir=$(prefix)/share
mandir=$(datarootdir)/man
man1dir=$(mandir)/man1


.PHONY: all clean install installdirs

all: $(EXECUTABLE)

$(EXECUTABLE): $(EXECUTABLE).c config.h

clean:
	rm -f $(EXECUTABLE)

install: $(EXECUTABLE) installdirs
	$(INSTALL_PROGRAM) $(EXECUTABLE) $(DESTDIR)$(bindir)/$(EXECUTABLE)

installdirs:
	mkdir -p $(DESTDIR)$(bindir) $(DESTDIR)$(man1dir)
