EXEC = Bonsai
VERSION = 0.1

TOPDIR := $(shell pwd)
SRCDIR := $(TOPDIR)/src
BINDIR := $(TOPDIR)/bin
OBJDIR := $(TOPDIR)/obj
HDRDIR := $(TOPDIR)/lib

SOURCES := $(shell find $(SRCDIR) -name "*.c") 
OBJECTS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

X11INC = /usr/include/X11
X11LIB = /usr/lib/X11

INCS = -I$(X11INC) -I$(HDRDIR)
LIBS = -L$(X11LIB) -lX11

CFLAGS = -std=c2x -pedantic -Wall $(INCS) $(LIBS) -DVERSION=\"$(VERSION)\"
