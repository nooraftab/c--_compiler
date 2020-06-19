IDIR=../includes
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj

_DEPS=lexer.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=main.o lexer.o lexemitter.o lexerror.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

lexer: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

