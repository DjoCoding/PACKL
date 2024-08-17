CC = gcc

CFLAGS = -Wall -Werror -g

SRCDIR = src
OBJDIR = obj
TOOLSDIR = tools
BIN = bin

ENTRY = packlc.c

SOURCES = $(ENTRY) $(wildcard $(TOOLSDIR)/*.c)  $(wildcard $(SRCDIR)/*.c)

OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

EXEC = packlc

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^
	@mkdir -p $(BIN)
	mv $(EXEC) $(BIN)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJDIR)/*.o $(EXEC)

.PHONY: all clean
