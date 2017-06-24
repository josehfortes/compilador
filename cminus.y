/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static int savedLineNo;  /* ditto */
static int savedNumber;
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void);


%}

%token IF ELSE INT RETURN VOID WHILE PLUS MINUS TIMES OVER LT LE EQ MT ME EQEQ NEQ SEMI COL LPAREN RPAREN LCOLC RCOLC LK RK
%token ID NUM
%token ERROR

%% /* Grammar for C- */

program           : declaracao_lista
                      { savedTree = $1; }
                  ;
declaracao_lista  : declaracao_lista declaracao
		  { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1;
		   }
                     else $$ = $2;
                  }
                  | declaracao { $$ = $1; }
                  ;
declaracao        : var_declaracao { $$ = $1; }
                  | fun_declaracao { $$ = $1; }
                  ;
var_declaracao    : tipo_especificador id SEMI /* declaração de variavel, o tipo(int ou void) é salvo no nó e o filho recebe o nome */
		  {
	            $$ = $1;
		    $$->nodekind = StmtK;
                    $$->kind.stmt = VarK; //tipo - variavel
		    $2->attr.type = "variavel";
		    $$->child[0] = $2; // filho recebendo "id"
		  }
                  | tipo_especificador id LCOLC num RCOLC SEMI /* declaração de vetores, o tamanho é salvo em $$->attr.val, o nó fica comotipo especificador(int ou void) e o seu filho recebe o nome */
		  {
		    $$ = $1;
		    $$->nodekind = StmtK;
            $$->kind.stmt = VectorK; //tipo - vetor
            $2->attr.type = "vetor";
		    $$->child[0] = $2;
		    $$->attr.val = $4->attr.val; //tamanho do vetor salvo no próprio nó
		  }
                  ;
tipo_especificador: INT
		  {
		    $$ = newExpNode(IntK);
		    $$->attr.name = "Integer";

		  }
                  | VOID
		  {
		    $$ = newExpNode(VoidK);
		    $$->attr.name = "Void";
		  }
                  ;
fun_declaracao    : tipo_especificador id LPAREN params RPAREN composto_decl //declaração de função
		  {
		    $$ = $1;
		    $$->nodekind = StmtK;
            $$->kind.stmt = FuncaoK;
            $2->attr.type = "funcao";
		    $$->child[0] = $2;
			//salvando os filhos do nó como parametros(primeiro) e tudo dentro da funcao(composto) em segundo
		    $2->child[0] = $4;
		    $2->child[1] = $6;
		  }
                  ;
params            : param_lista {$$ = $1;}
                  | VOID
		  {
		    $$ = newExpNode(VoidK);
		    $$->attr.name = "Void";
		  }
                  ;
param_lista       : param_lista COL param
		  { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $3;
                     $$ = $1;
		   }
                     else $$ = $3;
                  }
                  | param {$$ = $1;}
                  ;
param             : tipo_especificador id
		  {
		    $$ = $1;
		    $$->nodekind = StmtK;
                    $$->kind.stmt = VarParK; //tipo - variavel
                    $2->attr.type = "variavel";
		    $$->child[0] = $2; // filho recebendo "id"
		  }
                  | tipo_especificador id LCOLC RCOLC //quando
		  {
		    $$ = $1;
		    $$->nodekind = StmtK;
                    $$->kind.stmt = VectorK; //tipo - variavel
                    $2->attr.type = "vetor";
		    $$->child[0] = $2; // filho recebendo "id"
		  }
                  ;
composto_decl     : LK local_declaracoes statement_lista RK
		  {
		    $$ = newStmtNode(CompK);
		    $$->child[0] = $2; /* declaracoes locais */
                    $$->child[1] = $3; /* statements */
		  }
		  | LK local_declaracoes RK
		  { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1;
		   }
                     else $$ = $2;
          }

		  | LK statement_lista RK
		  { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1;
		   }
                     else $$ = $2;
                  }
		  | LK RK { $$ = NULL; }
                  ;
local_declaracoes : local_declaracoes var_declaracao
		  { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1;
		   }
                     else $$ = $2;
                  }
                  | var_declaracao { $$ = $1; }
                  ;
statement_lista   : statement_lista statement
		  { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1;
		   }
                     else $$ = $2;
                  }
                  | statement { $$ = $1; }
                  ;
statement         : expressao_decl { $$ = $1; }
                  | composto_decl { $$ = $1; }
                  | selecao_decl { $$ = $1; }
                  | iteracao_decl { $$ = $1; }
                  | retorno_decl { $$ = $1; }
                  ;
expressao_decl    : expressao SEMI { $$ = $1; }
                  | SEMI { $$ = NULL; }
                  ;
selecao_decl      : IF LPAREN expressao RPAREN statement
		  {
		    $$ = newStmtNode(IfK);
		    $$->child[0] = $3;
                    $$->child[1] = $5;
		  }
                  | IF LPAREN expressao RPAREN statement ELSE statement
		  {
		    $$ = newStmtNode(IfK);
		    $$->child[0] = $3;
                    $$->child[1] = $5;
		    $$->child[2] = $7;
		  }
                  ;
iteracao_decl     : WHILE LPAREN expressao RPAREN statement
		  {
		    $$ = newStmtNode(WhileK);
		    $$->child[0] = $3;
                    $$->child[1] = $5;
		  }
                  ;
retorno_decl      : RETURN SEMI
		  {
		    $$ = newStmtNode(ReturnK);
		    $$->child[0] = NULL;
		  }
                  | RETURN expressao SEMI
		  {
		    $$ = newStmtNode(ReturnK);
		    $$->child[0] = $2;
		  }
                  ;
expressao         : var EQ expressao
                 { $$ = newExpNode(OpK);
		   $$->attr.op = EQ;
                   $$->child[0] = $1;
                   $$->child[1] = $3;

                 }
                  | simples_expressao { $$ = $1; }
                  ;
var               : id
		  { $$ = $1; $$->attr.type = "id"; $$->attr.typeVar = "var";}
                  | id LCOLC expressao RCOLC
		  {
		    $$ = $1;
        $$->attr.type = "id";
        $$->attr.typeVar = "vector";
		    $$->child[0] = $3;
		  }
                  ;
simples_expressao : soma_expressao relacional soma_expressao
		  {
		    $$ = $2;
		    $$->child[0] = $1;
		    $$->child[1] = $3;
		  }
                  | soma_expressao
		  {
		    $$ = $1;
		  }
                  ;
relacional        : LE
		  {
		    $$ = newExpNode(OpK);
		    $$->attr.op = LE;
		  }
                  | LT
		  {
		    $$ = newExpNode(OpK);
		    $$->attr.op = LT;
		  }
                  | MT
		  {
		    $$ = newExpNode(OpK);
		    $$->attr.op = MT;
		  }
                  | ME
		  {
		    $$ = newExpNode(OpK);
		    $$->attr.op = ME;
		  }
                  | EQEQ
		  {
		    $$ = newExpNode(OpK);
		    $$->attr.op = EQEQ;
		  }
                  | NEQ
		  {
		    $$ = newExpNode(OpK);
		    $$->attr.op = NEQ;
		  }
                  ;
soma_expressao    : soma_expressao soma termo
		  {
		    $$ = $2;
		    $$->child[0] = $1;
		    $$->child[1] = $3;
		  }
                  | termo
		  {
		    $$ = $1;
		  }
                  ;
soma              : PLUS
		  {
		    $$ = newExpNode(OpK);
		    $$->attr.op = PLUS;
		  }
                  | MINUS
		  {
		    $$ = newExpNode(OpK);
		    $$->attr.op = MINUS;
		  }
                  ;
termo             : termo mult fator
		  {
		    $$ = $2;
		    $$->child[0] = $1;
		    $$->child[1] = $3;
		  }
                  | fator
		  {
		    $$ = $1;
		  }
                  ;
mult              : TIMES
		  {
		    $$ = newExpNode(OpK);
		    $$->attr.op = TIMES;
		  }
                  | OVER
		  {
		    $$ = newExpNode(OpK);
		    $$->attr.op = OVER;
		  }
                  ;
fator             : LPAREN expressao RPAREN{ $$ = $2; }
                  | var{ $$ = $1; }
                  | ativacao{ $$ = $1; }
                  | num{ $$ = $1; }
                  ;
ativacao          : id LPAREN arg_lista RPAREN
		  {
		    $$ = $1;
		    $$->kind.exp = AtivK;
		    $$->attr.type = "funcao";
		    $$->child[0] = $3;
		  }
		  | id LPAREN RPAREN
                  ;
arg_lista         : arg_lista COL expressao
		  { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $3;
                     $$ = $1;
		   }
                     else $$ = $3;
                  }
                  | expressao { $$ = $1; }
                  ;

id 		  : ID
		  {
		    $$ = newExpNode(IdK);
                    $$->attr.name = copyString(tokenString);
                    $$->attr.type = "chamadaFuncao";
		  }
		  ;
num		  : NUM
		  {
		   $$ = newExpNode(ConstK);
           $$->attr.val = atoi(tokenString);
		   $$->attr.type = "Integer";
		  }
		  ;

%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}
