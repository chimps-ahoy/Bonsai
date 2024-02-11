include config.mk

# fo debugin
CFLAGS += -g -DDEBUG

$(EXEC) : $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $(BINDIR)/$@

$(OBJDIR)%.o : $(SRCDIR)%.c
	$(CC) $(CFLAGS) -c $< -o $@ 

clean :
	rm -f $(BINDIR)/* $(OBJDIR)/*
