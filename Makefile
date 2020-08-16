CC=gcc
PCC=g++
CFLAGS=-std=c11 -g3 -O3
CPPFLAGS=-std=c++11 -g3 -O0

SDIR=.
ODIR=.obj
PODIR=.pobj
BDIR=bin

SRC=$(wildcard $(SDIR)/*.c)
OBJ=$(patsubst $(SDIR)/%.c,$(ODIR)/%.o,$(SRC))
EXES=$(patsubst $(SDIR)/%.c,$(BDIR)/%,$(SRC))

PSRC=$(wildcard $(SDIR)/*.cpp)
POBJ=$(patsubst $(SDIR)/%.cpp,$(PODIR)/%.o,$(PSRC))
PEXES=$(patsubst $(SDIR)/%.cpp,$(BDIR)/%,$(PSRC))

$(shell mkdir -p $(ODIR))
$(shell mkdir -p $(PODIR))
$(shell mkdir -p $(BDIR))

all: $(EXES) $(OBJ) $(PEXES) $(POBJ)

$(BDIR)/%: $(ODIR)/%.o
	$(CC) $(CFLAGS) $< -o $@

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BDIR)/%: $(PODIR)/%.o
	$(PCC) $(CPPFLAGS) $< -o $@

$(PODIR)/%.o: $(SDIR)/%.cpp
	$(PCC) $(CPPFLAGS) -c $< -o $@

-include $(wildcard $(ODIR)/*.d)

.PHONY: clean
clean:
	-rm -rf $(ODIR) $(PODIR) $(BDIR) $(EXE)
