EXEC = Bonsai
VERSION = 0.0

TOPDIR != pwd
SRCDIR ::= $(TOPDIR)/src
BINDIR ::= $(TOPDIR)/bin
OBJDIR ::= $(TOPDIR)/obj
HDRDIR ::= $(TOPDIR)/lib
LOGDIR ::= $(TOPDIR)/logs

SOURCES != find $(SRCDIR) -name "*.c"
OBJECTS != echo $(SOURCES) | sed -e 's,\.c,.o,g' -e s,$(SRCDIR),$(OBJDIR),g

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

INCS = -I$(HDRDIR)
LIBS = -lswc -lwayland-server -lc

CC=/bin/cc
CFLAGS = -std=c2x -pedantic -Wall $(INCS) $(LIBS) -DVERSION=\"$(VERSION)\" -D_POSIX_C_SOURCE=200112L
