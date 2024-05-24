include config.mk

# fo debugin
CFLAGS += -g -DDEBUG

$(EXEC) : $(OBJECTS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $(BINDIR)/$@

$(OBJDIR)%.o : $(SRCDIR)%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@ 

clean :
	rm -f $(BINDIR)/* $(OBJDIR)/*
