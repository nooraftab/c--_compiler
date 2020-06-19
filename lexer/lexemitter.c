#include "lexer.h"

/** Displays tokens from parsing through lexemes **/

void lexer_emit(int t, char *tval) {  

  switch(t) {
    case READ:
      printf("READ\n"); 
      break;
    case IF:
      printf("IF\n"); 
      break;
    case ELSE:
      printf("ELSE\n"); 
      break;
    case INTTOK:
      printf("INTTOK\n"); 
      break;
    case CHARTOK:
      printf("CHARTOK\n"); 
      break;
    case BREAK:
      printf("BREAK\n"); 
      break;
    case WRITE:
      printf("WRITE\n"); 
      break;
    case WRITELN:
      printf("WRITELN\n"); 
      break;
    case WHILE:
      printf("WHILE\n"); 
      break;
    case RETURN:
      printf("RETURN\n"); 
      break;
    case ID:
      printf("ID.%s\n", tval); 
      break;
    case NUM:
      printf("NUM.%s\n", tval); 
      break;
    case FLOAT: 
      printf("FLOAT.%s\n", tval); 
      break;
    //Punctuation 
    case SEMICOLON:
      printf("SEMICOLON\n"); 
      break;
    case LBRACK:
      printf("LBRACK\n");  
      break;
    case RBRACK: 
      printf("RBRACK\n"); 
      break;
    case LBRACE:
      printf("LBRACE\n");  
      break;
    case RBRACE: 
      printf("RBRACE\n"); 
      break;
    case LPAREN:
      printf("LPAREN\n");  
      break;
    case RPAREN: 
      printf("RPAREN\n"); 
      break;
    case COMMA:
      printf("COMMA\n");
    break;

    //Operators
    case EQ:
      printf("EQ\n"); 
      break;
    case ASSIGN: 
      printf("ASSIGN\n"); 
      break;
    case NEQ: 
      printf("NEQ\n"); 
      break;
    case NEG:
      printf("NEG\n");  
      break;
    case SUB: 
      printf("SUB\n"); 
      break;
    case ADD: 
      printf("ADD\n"); 
      break;
    case MULT:
      printf("MULT\n");  
      break;
    case DIV: 
      printf("DIV\n"); 
      break;
    case LESS:
      printf("LESS\n");  
      break;
    case LEQ:
      printf("LEQ\n");  
      break;
    case GREAT: 
      printf("GREAT\n"); 
      break;
    case GEQ: 
      printf("GEQ\n"); 
      break;
    case AND: 
      printf("AND\n"); 
      break;
    case OR:
      printf("OR\n");  
      break;
    case DONE:
      printf("DONE\n");
      break;
    case LEXERROR:
      printf("You're in some ~trouble~. Look below.\n");
      break;
    default:
      printf("??? Something happened and I don't like it."); 
      break;
  }
}