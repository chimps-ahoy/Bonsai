include config.mk

# fo debugin
CFLAGS += -g -lcurses

$(EXEC) : $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $(BINDIR)/$@

$(OBJDIR)%.o : $(SRCDIR)%.c
	$(CC) $(CFLAGS) -c $< -o $@ 

clean :
	rm -f $(BINDIR)/* $(OBJDIR)/*
