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

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

INCS = -I$(X11INC) -I$(HDRDIR)
LIBS = -L$(X11LIB) -lX11

CFLAGS = -std=c2x -pedantic -Wall $(INCS) $(LIBS) -DVERSION=\"$(VERSION)\"
