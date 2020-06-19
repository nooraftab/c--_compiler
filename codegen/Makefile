#
# compiles mycc, a compiler for C-- 
#
# make depend: automatically build .o file dependencies
# make: build mycc
# make clean: removes all .o and executable files
# @author: Valerie Barr
#

CC = gcc
CFLAGS = -Wall -g

LFLAGS = 
LIBS = 

INCLUDES =  -I../includes

# add additional source files here
SRCS = ../ast/ast.c ../lexer/lexer.c ../lexer/lexemitter.c  ../lexer/lexerror.c\
       ../parser/parser.c ../symtab/symtab.c ../symtab/symtaberror.c traversalmechanics.c codetraversal.c traversaltotable.c tablemechanics.c codegenerror.c main.c 

OBJS = $(SRCS:.c=.o)


MAIN = mycc

# generic part of makefile...don't change this
all:    $(MAIN) 
	@echo  Simple parser named parser has been compiled

$(MAIN):  $(OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS) 

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it

