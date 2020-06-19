#
# compiles parser, a parser for C-- 
#
# make depend: automatically build .o file dependencies
# make: build parser
# make pure: build a purified version of parser
# make clean: removes all .o and executable files
#

CC = gcc
CFLAGS = -Wall -g

LFLAGS = 
LIBS = 

INCLUDES =  -I../includes

# add additional source files here
SRCS = ../ast/ast.c ../lexer/lexer.c parser.c main.c ../lexer/lexemitter.c ../lexer/lexerror.c

OBJS = $(SRCS:.c=.o)


MAIN = parser

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

