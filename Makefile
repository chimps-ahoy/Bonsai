include config.mk

#TODO: these will have to be changed for release to put files in /usr/local/bin/ instead of just running it all from
#current directory!

run : $(EXEC)
	$(BINDIR)/$(EXEC) 2>$(LOGDIR)/"$(shell date +"%F@%H:%M:%S").log"

debug : CFLAGS += -g -DDEBUG
debug : $(EXEC)

$(EXEC) : bin
	printf "#!/bin/sh\n\nswc-launch $(BINDIR)/$(EXEC)_bin" > $(BINDIR)/$(EXEC)
	chmod +x $(BINDIR)/$(EXEC)

bin : $(OBJECTS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $(BINDIR)/$(EXEC)_$@

$(OBJDIR)%.o : $(SRCDIR)%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@ 

clean :
	rm -f $(BINDIR)/* $(OBJDIR)/* $(LOGDIR)/*
