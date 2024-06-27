EXEC    = Bonsai
VERSION = 0.0

TOPDIR != pwd
SRCDIR ::= $(TOPDIR)/src
HDRDIR ::= $(TOPDIR)/lib
BINDIR ::= $(TOPDIR)/bin
LOGDIR ::= $(TOPDIR)/logs

SOURCES != find $(SRCDIR) -name "*.c"
OBJECTS != echo $(SOURCES) | sed -e 's,\.c,.o,g'
HDRS    != find $(HDRDIR) -name "*.h"

PREFIX    = /usr/local
MANPREFIX = $(PREFIX)/share/man

INCS = -I$(HDRDIR)
LIBS = -lswc -lwayland-server -lc

CC     =/bin/cc
CFLAGS = -std=c2x -pedantic -Wall $(INCS) $(LIBS) -DVERSION=\"$(VERSION)\" \
		 -D_POSIX_C_SOURCE=200112L $(DEBUG)
