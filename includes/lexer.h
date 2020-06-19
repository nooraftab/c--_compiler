// @author: Valerie Barr
/****** lexer.h ********************************************************/

#ifndef _LEXER_H
#define _LEXER_H

// INCLUDES:
#include <stdio.h>


// Constants used by the lexer:
#define MAXLEXSIZE    128            // Maximum length of a lexeme
#define NONE           -1
#define LEXERROR       -1 

// Enums for possible token values. By Noor Aftab
typedef enum {STARTTOKEN, 
			  ID, NUM, FLOAT, 
			  READ, IF, ELSE, INTTOK, CHARTOK,
			  BREAK, WRITE, WRITELN, WHILE,
			  RETURN,
			  //Some punctuation
			  LBRACK, RBRACK, 
			  //Other Operators
			  EQ, ASSIGN, NEQ, NEG, 
			  SUB, ADD, MULT, DIV,
			  LESS, LEQ, GREAT, GEQ, 
			  AND, OR, 
                         // Indicated lexical analyzer is done
              ENDTOKEN,
              // Punctuation that isn't as necessary for tree-building
			  SEMICOLON, LBRACE, RBRACE, LPAREN, RPAREN, COMMA,
			  DONE
			  } tokenT;

extern char lexbuf[];  // Used for stepping through lexeme
extern char  tokenval[];  // Token's value (name for IDs, value for nums etc.)

// Line # in source code
extern int  src_lineno;    
extern char lexer_error_message[]; 

// Function prototypes
extern int lexan(FILE *fd);
extern void lexer_emit(int t, char* tval); //Prints token + value
void lexer_error(char *m, int lineno); //Prints error messages on LEXERROR

/** Debugging help below **/

// MACROS DEFINITIONS:  in general, use functions rather than macros as the
//      compiler can check parameter type for functions
// macros for debugging output

// uncomment DEBUG_LEXER #define to enable debug output
//#define DEBUG_LEXER     1
#ifdef DEBUG_LEXER  // DEBUG_LEXER on:
#define lexer_debug0(str)            printf(str)           // 1 string arg
#define lexer_debug1(fmtstr, arg1)   printf(fmtstr,arg1)   // format str & 1 arg     
#else  // DEBUG_LEXER off:
#define lexer_debug0(str)        
#define lexer_debug1(fmtstr, arg1)  
#endif

#endif
