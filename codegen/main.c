/*
 *  Main function for C-- compiler
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "traversaltotable.h"
#include "codetraversal.h"
#include "parser.h"
#include "symtab.h"
#include "ast.h"

int main(int argc, char *argv[]) {

  FILE *in = 0, *out = 0;

  if(argc != 3) { 
    printf("usage: mycc  filename.c--  filename.s\n");
    exit(1);
  }
  if(!(in = fopen(argv[1], "rw")) ) {
    perror("no such file\n");
    exit(1);
  }
  if(!(out = fopen(argv[2], "w")) ) {
    perror("opening output file faild\n");
    exit(1);
  }

  inFile = in;
  outFile = out;

  parse(in);   
  init_symtab_stack(); 
  traverse_and_generate_code(); 
  output_code_table_to_file(out);                              
  
  //Free up heap memory we used for our data structures
  destroy_code_table();
  destroy_ast(&ast_tree);
  destroy_symtab_stack();

  fclose(in);
  fclose(out);
  exit(0);     
}
