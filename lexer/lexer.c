/* 
  Noor's Lexical Analyzer for C--!
 */

#include <stdlib.h>
#include <assert.h>
#include "lexer.h"
#include <ctype.h>
#include <string.h>

char lexbuf[MAXLEXSIZE];  //Stores current lexeme
char tokenval[MAXLEXSIZE];  //Stores current token's value

//Variables that help for debugging/catching errors
int  src_lineno=1;  
char lexer_error_message[MAXLEXSIZE];   

/** Handles punctuation, simple operators, & figures out where to go otherwise **/
static int start_state();

/** Functions for numbers, IDs, keywords, & whitespace **/
static int ws_state();
static int id_state();
static int check_keyword_state();
static int num_state(); 
static int float_state();
static int char_literal_num_state();

/** Operators that can take a few routes **/
static int eq_or_assign_state();
static int neg_or_neq_state();
static int less_or_leq_state();
static int great_or_geq_state();
static int and_state();
static int or_state();

/** Comments! **/
static int comment_or_div_state();
static int star_comment_state();
static int check_star_comment_end_state();
static int slash_comment_state();

/***************************************************************************/

/* Kickstarts lexeme parsing!*/
int lexan(FILE *fd) { 
  //fgetc so we can parse character by character 
  *lexbuf = (char) fgetc(fd); 
  strcpy(tokenval, ""); //Initialize tokenval

   if (*lexbuf == EOF) {
    return DONE;
  }
  return start_state(fd);
}

/** Start state! **/
static int start_state(FILE *fd) {

  switch(*lexbuf) {
    //Punctuation
    case ';':
      return SEMICOLON;
    case '[':
      return LBRACK;
    case ']':
      return RBRACK;
    case '{':
      return LBRACE;
    case '}':
      return RBRACE;
    case '(':
      return LPAREN;
    case ')':
      return RPAREN;
    case ',':
      return COMMA;

    //Operators with more choice, or where lexical errors can occur
    case '=':
      return eq_or_assign_state(fd);
    case '!':
      return neg_or_neq_state(fd);
    case '<':
      return less_or_leq_state(fd);
    case '>':
      return great_or_geq_state(fd);
    case '&':
      return and_state(fd);
    case '|':
      return or_state(fd);

    //Other "simpler" operators
    case '-':
      return SUB;
    case '+':
      return ADD;
    case '*':
      return MULT;
    //Need to check if a '/' is for division or comments
    case '/': 
      return comment_or_div_state(fd);
    //Char literals should be treated like numbers 
    case '\'':
      return char_literal_num_state(fd);
  }

  /** whitespace, IDs/keywords, and numbers/floats are more intensive **/

  int isSpace = isspace(*lexbuf);
  int isAlpha = isalpha(*lexbuf);
  int isDigit = isdigit(*lexbuf);

  if (isSpace) {
    return ws_state(fd);
  } else if (isAlpha) { 
    return id_state(fd);
  } else if (isDigit) {
    return num_state(fd);
  }
  strcpy(lexer_error_message, "You entered something weird.\n");
  return LEXERROR;
}

/** Scans through whitespace (increments src_lineno as needed) **/
static int ws_state(FILE *fd) { 
  while (isspace(*lexbuf)) {
    if (*lexbuf == '\n') {
      src_lineno++;
    }
    *lexbuf = (char) fgetc(fd);
  }
  ungetc(*lexbuf, fd); 
  return lexan(fd);
}

/** Parses IDs, checks if they happen to be keywords **/
static int id_state(FILE *fd) {
  while (isalnum(*lexbuf) || *lexbuf == '_') {
    strcat(tokenval, lexbuf);
    *lexbuf = (char) fgetc(fd);
  }
  ungetc(*lexbuf, fd);

  return check_keyword_state(fd);
}

/** Checks if an ID is a keyword and returns accordingly  **/
static int check_keyword_state(FILE *fd) {
  if (strcmp(tokenval, "return") == 0) {
    return RETURN;
  } else if (strcmp(tokenval, "read") == 0) {
    return READ;
  } else if (strcmp(tokenval, "if") == 0) {
    return IF;
  } else if (strcmp(tokenval, "else") == 0) {
    return ELSE;
  } else if (strcmp(tokenval, "int") == 0) {
    return INTTOK;
  } else if (strcmp(tokenval, "char") == 0) {
    return CHARTOK;
  } else if (strcmp(tokenval, "break") == 0) {
    return BREAK;
  } else if (strcmp(tokenval, "write") == 0) {
    return WRITE;
  } else if (strcmp(tokenval, "writeln") == 0) {
    return WRITELN;
  } else if (strcmp(tokenval, "while") == 0) {
    return WHILE;
  }

  return ID;
}

/** Parses plain-ol numbers **/
static int num_state(FILE *fd) {
  while (isdigit(*lexbuf)) {
    strcat(tokenval, lexbuf);
    *lexbuf = (char) fgetc(fd);
  }

  if (*lexbuf == '.') { //Float check!
    strcat(tokenval, lexbuf);
    return float_state(fd);
  }

  ungetc(*lexbuf, fd);
  return NUM;
}

/** Checks for floats. **/
static int float_state(FILE *fd) {
  *lexbuf = (char) fgetc(fd);

  //Needs at least one digit after the '.'
  if (!isdigit(*lexbuf)) {
    strcpy(lexer_error_message, "Floats should only have digits in them.");
    return LEXERROR;
  }

  while (isdigit(*lexbuf)) {
    strcat(tokenval, lexbuf);
    *lexbuf = (char) fgetc(fd);
  }
  ungetc(*lexbuf, fd);
  return FLOAT;

}

/** Parses for char-literal numbers **/
static int char_literal_num_state(FILE *fd) {
  *lexbuf = (char) fgetc(fd);
  strcat(tokenval, lexbuf);

  if (isascii(*lexbuf)) {
    if (*lexbuf == '\\') {
       *lexbuf = (char) fgetc(fd);
      if (*lexbuf == 't') {
        snprintf(tokenval, MAXLEXSIZE, "%d", '\t');
      } else if (*lexbuf == 'f') {
        snprintf(tokenval, MAXLEXSIZE, "%d", '\f');
      } else if (*lexbuf == 'n') {
        snprintf(tokenval, MAXLEXSIZE, "%d", '\n');
      } else if (*lexbuf == 'r') {
        snprintf(tokenval, MAXLEXSIZE, "%d", '\r');
      } else if (*lexbuf == 'v') {
        snprintf(tokenval, MAXLEXSIZE, "%d", '\v');
      } else {
        strcpy(lexer_error_message, "Invalid control character!");
        return LEXERROR;
      }
      *lexbuf = (char) fgetc(fd);
      return NUM;
    }

    *lexbuf = (char) fgetc (fd);
    if (*lexbuf == '\'') {
      //Sets token value to be decimal value of ASCII character
      int literalValue = (int) *tokenval;
      snprintf(tokenval, MAXLEXSIZE, "%d", literalValue);
      return NUM;
    } 

    strcpy(lexer_error_message, "Char literal goes like: 'a' or '\\a'");
    return LEXERROR; //If end quote isn't after one character 
  }
  strcpy(lexer_error_message, "Stick to ASCII please.");
  return LEXERROR;  //If program just has some really weird thing .
}

/** Checks if '=' is party of EQ or ASSIGN operators **/
static int eq_or_assign_state(FILE *fd) {
  *lexbuf = (char) fgetc(fd);
  if (*lexbuf == '=') {
    return EQ;
  } else {
    ungetc(*lexbuf, fd);
    return ASSIGN;
  }
}

/** Checks if '!' is party of NEG or NEQ operators **/
static int neg_or_neq_state(FILE *fd) {
  *lexbuf = (char) fgetc(fd);
  if (*lexbuf == '=') {
    return NEQ;

  } else {
    ungetc(*lexbuf, fd);
    return NEG;
  }
}

/** Checks if '<' is party of LESS or LEQ operators **/
static int less_or_leq_state(FILE *fd) {
  *lexbuf = (char) fgetc(fd);
  if (*lexbuf == '=') {
    return LEQ;
  } else {    
    ungetc(*lexbuf, fd);
    return LESS;
  }
}

/** Checks if '>' is party of GREAT or GEQ operators **/
static int great_or_geq_state(FILE *fd) {
  *lexbuf = (char) fgetc(fd);
  if (*lexbuf == '=') {
    return GEQ;
  } else {
    ungetc(*lexbuf, fd);
    return GREAT;
  }
}

/** Makes sure we only have '&&' in program and not '&' **/
static int and_state(FILE *fd) {
  *lexbuf = (char) fgetc(fd);
  if (*lexbuf == '&') {
    return AND;
  } else {
    printf("Line %d: %s\n", src_lineno, "'And' goes like this: &&. Replaced with correct token, but please fix it!");
    return AND;
  }
}

/** Makes sure we only have '||' in program and not '|' **/
static int or_state(FILE *fd) {
  *lexbuf = (char) fgetc(fd);
  if (*lexbuf == '|') {
    return OR;
  } else {
    printf("Line %d: %s\n", src_lineno, "'OR' goes like this: ||. Replaced with correct token, but please fix it!");
    return OR;
  }
}

/** Checks if '/' is for a comment or the division operator **/
static int comment_or_div_state(FILE *fd) {
  *lexbuf = (char) fgetc(fd);

  if (*lexbuf == '*') { /* Comment */
    return star_comment_state(fd);
  } else if (*lexbuf == '/') { //Comment 
    return slash_comment_state(fd);
  } else { //Division operator 
    ungetc(*lexbuf, fd);
    return DIV;
  }
}

/** Goes through a star-comment, checking throughout for its end**/
static int star_comment_state(FILE *fd) {
  *lexbuf = (char) fgetc(fd);

  while (*lexbuf != '*' && *lexbuf != EOF) {
    if (*lexbuf == '\n') { //Increment src_lineno as needed
      src_lineno++;
    }
    *lexbuf = (char) fgetc(fd);
  }

  if (*lexbuf == EOF) { //Shouldn't hit EOF before comment ends
    strcpy(lexer_error_message, "Did you forget to end your comment?");
    return LEXERROR;
  }
  return check_star_comment_end_state(fd);

}

/** Checks if star comment ended, or if it was a false alarm **/
static int check_star_comment_end_state(FILE *fd) {
  *lexbuf = (char) fgetc(fd);

  if (*lexbuf == '/') {
    return lexan(fd);
  } else {
    ungetc(*lexbuf, fd);
    return star_comment_state(fd);
  }
}

/** For '//' comments, go through until a new-line **/
static int slash_comment_state(FILE *fd) {
  *lexbuf = (char) fgetc(fd);

  while (*lexbuf != '\n') {
    *lexbuf = (char) fgetc(fd);
  }
  src_lineno++;
  return lexan(fd);
}