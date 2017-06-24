/****************************************************/
/* File: tiny.l                                     */
/* Lex specification for TINY                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

%{
#include "globals.h"
#include "util.h"
#include "scan.h"
/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];
int verifica;
static int yylex(void);
%}

digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  {letter}+
newline     \n
whitespace  [ \t\r]+

%%

"if"            {return IF;}
"else"          {return ELSE;}
"int"           {return INT;}
"return"        {return RETURN;}
"void"          {return VOID;}
"while"         {return WHILE;}

"+"             {return PLUS;}
"-"             {return MINUS;}
"*"             {return TIMES;}
"/"             {return OVER;}
"<"             {return LT;}
"<="             {return LE;}
"="             {return EQ;}
">"             {return MT;}
">="             {return ME;}
"=="             {return EQEQ;}
"!="             {return NEQ;}
";"             {return SEMI;}
","             {return COL;}
"("             {return LPAREN;}
")"             {return RPAREN;}
"["             {return LCOLC;}
"]"             {return RCOLC;}
"{"             {return LK;}
"}"             {return RK;}


{number}        {return NUM;}
{identifier}    {return ID;}
{newline}       {lineno++;}
{whitespace}    {/* skip whitespace */}

"/*"             { char c;
		   int verifica = 0;
                  do
                  {
		    c = input();
                    if (c == EOF) break;
                    if (c == '\n') lineno++;
		    if (c == '*'){
		       c = input();
		       if (c == '/')verifica = 1;
		    }
                  } while (verifica != 1);
                }
.               {return ERROR;}

%%

TokenType getToken(void)
{ static int firstTime = TRUE;
  TokenType currentToken;
  if (firstTime)
  { firstTime = FALSE;
    lineno++;
    yyin = source;
    yyout = listing;
  }
  currentToken = yylex();
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan) {
    fprintf(listing,"\t%d: ",lineno);
    printToken(currentToken,tokenString);
  }
  return currentToken;
}
