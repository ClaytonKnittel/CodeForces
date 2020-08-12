CC=gcc
CFLAGS=-std=c11 -g3 -O0

SDIR=.
ODIR=.obj
BDIR=bin

SRC=$(wildcard $(SDIR)/*.c)
OBJ=$(patsubst $(SDIR)/%.c,$(ODIR)/%.o,$(SRC))
EXES=$(patsubst $(SDIR)/%.c,$(BDIR)/%,$(SRC))

$(shell mkdir -p $(ODIR))
$(shell mkdir -p $(BDIR))

all: $(EXES) $(OBJ)

$(BDIR)/%: $(ODIR)/%.o
	$(CC) $(CFLAGS) $< -o $@

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(wildcard $(ODIR)/*.d)

.PHONY: clean
clean:
	-rm -rf $(ODIR) $(BDIR) $(EXE)
