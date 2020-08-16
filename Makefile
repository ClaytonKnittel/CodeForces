CC=gcc
PCC=g++
CFLAGS=-std=c11 -g3 -O3
CPPFLAGS=-std=c++11 -g3 -O0

SDIR=.
ODIR=.obj
PODIR=.pobj
BDIR=bin

SRC=$(shell find $(SDIR) -type f -name '*.c')
OBJ=$(patsubst $(SDIR)/%.c,$(ODIR)/%.o,$(SRC))
EXES=$(patsubst $(SDIR)/%.c,$(BDIR)/%,$(SRC))

PSRC=$(shell find $(SDIR) -type f -name '*.cpp')
POBJ=$(patsubst $(SDIR)/%.cpp,$(PODIR)/%.o,$(PSRC))
PEXES=$(patsubst $(SDIR)/%.cpp,$(BDIR)/%,$(PSRC))

DIRS=$(shell find $(SDIR) -type f -name '*.c' -print0 | xargs -0 -n1 dirname | sort --unique)
OBJDIRS=$(patsubst $(SDIR)/%,$(ODIR)/%,$(DIRS))
PDIRS=$(shell find $(SDIR) -type f -name '*.cpp' -print0 | xargs -0 -n1 dirname | sort --unique)
POBJDIRS=$(patsubst $(SDIR)/%,$(PODIR)/%,$(PDIRS))
ALLDIRS=$(shell find $(SDIR) -type f -name '*.c*' -print0 | xargs -0 -n1 dirname | sort --unique)
BDIRS=$(patsubst $(SDIR)/%,$(BDIR)/%,$(ALLDIRS))

$(shell mkdir -p $(ODIR))
$(shell mkdir -p $(PODIR))
$(shell mkdir -p $(BDIR))
$(shell mkdir -p $(OBJDIRS))
$(shell mkdir -p $(POBJDIRS))
$(shell mkdir -p $(BDIRS))

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
