/*
 *  Main function for testing the C-- lexical analyzer.
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"

int main(int argc, char *argv[]) {
  tokenT token;
  FILE *fd;

  if(argc <= 1) { // No file inputted
      printf("usage: lexer infile.c--\n");
      exit(1);
  }
  fd = fopen(argv[1], "r");
  if(fd == 0) { // Error opening file
      printf("error opening file: %s\n", argv[1]);
      exit(1);
  }

  token = STARTTOKEN; //Default value

  // Magic happens here.
  while (token != DONE && token != LEXERROR) {
      token = lexan(fd); //Gets next token for next lexeme
      lexer_emit(token, tokenval); //Prints token + its value
  }

  //In case we encounter an error
  if (token == LEXERROR) {
    fclose(fd);
    lexer_error(lexer_error_message, src_lineno);	
    exit(1);  
  }

  fclose(fd);
  exit(0);     /*  successful termination  */
}
