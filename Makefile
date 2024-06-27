.POSIX:
include config.mk

#TODO: these will have to be changed for release to put files in /usr/local/bin/ instead of just running it all from
#current directory!

DATE != date +"%F@%H:%M:%S"
run : $(EXEC)
	$(BINDIR)/$(EXEC) 2>$(LOGDIR)/$(DATE).log

$(EXEC) : raw
	printf "#!/bin/sh\n\nswc-launch $(BINDIR)/$(EXEC)_"$< > $(BINDIR)/$@
	chmod +x $(BINDIR)/$@

raw : $(OBJECTS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(BINDIR)/$(EXEC)_$@

.c.o :
	$(CC) $(CFLAGS) -c $< -o $@ 

clean :
	rm -f $(BINDIR)/* $(SRCDIR)/*.o $(LOGDIR)/*
