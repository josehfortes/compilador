/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"
#include <string.h>

static char * nomeFuncao;
static int preserveLastScope = FALSE;

/* counter for variable memory locations */
static int location = 0;

/* Procedure traverse is a generic recursive
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc
 * in postorder to tree pointed to by t
 */



char* Str( ExpType type ) {
  switch( type ) {
      case Void:
      return( "Void" );
      break;
      case Integer:
      return( "Inteiro" );
      break;
      case Boolean:
      return( "Booleano" );
      break;
  }
}


static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

static int mainDeclarada = 0;

/* nullProc is a do-nothing procedure to
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

static void declError(TreeNode * t, char * message)
{ fprintf(listing,"Erro de declaração na linha %d: %s\n",t->lineno,message);
  Error = TRUE;
}


/* Procedure insertNode inserts
 * identifiers stored in t into
 * the symbol table
 */
static void insertNode( TreeNode * t)
{
  switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      {
	case CompK:
	break;
	case FuncaoK:
		nomeFuncao = t->child[0]->attr.name;
		//tipo da funcao: t->attr.name;

		if(strcmp(nomeFuncao, "main") == 0){
			mainDeclarada = 1; //main sendo declarada
		}

		if (st_lookup_top(nomeFuncao) >= 0) {
			if ((strcmp(nomeFuncao, "input") == 0) || (strcmp(nomeFuncao, "output") == 0)){

			}
			else{
				declError(t,"Função já declarada\n");
				break;
			}
		}



		t->attr.type = "Função";
		st_insert(nomeFuncao,t->attr.name,t->lineno,addLocation(),t); //insere na tabela,
		sc_push(sc_create(nomeFuncao));
		preserveLastScope = TRUE;
		//fprintf(listing, "Funcao - Nome: %s, Tipo: %s\n",t->child[0]->attr.name, t->attr.name);
		if (preserveLastScope) {
					preserveLastScope = FALSE;
			} else {
					Scope scope = sc_create(nomeFuncao);
				sc_push(scope);
			}
			t->scope = sc_top();

	break;
	case VectorK:
        //funcao que declara vetores
        //declaracao de variavel nova
        if (st_lookup_top(t->child[0]->attr.name) != -1){
           //variavel já está sendo usada anteriormente, apresenta erro
            declError(t, "Variavel ja foi usada anteriormente.");
        }
        else{
			t->attr.type = "Vetor";
			if(t->attr.val == NULL){
				t->par = 1;
				st_insert(t->child[0]->attr.name,t->attr.name,t->lineno,declaraVariavel(),t);
			}
            st_insert(t->child[0]->attr.name,t->attr.name,t->lineno,declaraVetor(t->attr.val),t);
        }
	break;
	case VarK:
        //funcao que declara variáveis
        //declaracao de variavel nova

        if (st_lookup_top(t->child[0]->attr.name) != -1){
           //variavel já está sendo usada anteriormente, apresenta erro
            declError(t, "Variavel ja foi usada anteriormente.");
        }
        else{
			//ainda é necessário verificar se existe uma função com o mesmo nome
			if (st_lookup(t->child[0]->attr.name) >= 0) {
				declError(t, "Variavel ja foi usada anteriormente como função.");
			}
			else{
				//insere a primeira variavel na tabela
				t->attr.type = "Variável";
				//verificar se é void
				if (strcmp(t->attr.name, "Void") == 0){
					//erro por ser void
					declError(t, "Variavel declarada como Void");
				}
				else{
					int decl = declaraVariavel();
					t->child[0]->scope = sc_top();
					t->scope = sc_top();
					st_insert(t->child[0]->attr.name,t->attr.name,t->lineno,decl,t);
				}
			}



        }
        //fprintf(listing, "ATRIBUICAO DE VARIAVEL TIPO: %s NOME: %s\n",t->attr.name, t->child[0]->attr.name);
    break;
    case VarParK:
        //funcao que declara variáveis DE FUNCOES
        //declaracao de variavel nova
        if (st_lookup_top(t->child[0]->attr.name) != -1){
           //variavel já está sendo usada anteriormente, apresenta erro
            declError(t, "Variavel ja foi usada anteriormente.;;");
        }
        else{
            //insere a primeira variavel na tabela
            t->attr.type = "Variável";
			t->par = 1;
 			t->child[0]->scope = sc_top();
			t->scope = sc_top();
            st_insert(t->child[0]->attr.name,t->attr.name,t->lineno,declaraVariavel(),t);
        }
	break;
	case ReturnK:
	break;
	case AssignK:
	break;
	case WhileK:
	break;
	case IfK:
	break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      {
	case OpK:

		if(t->attr.op == EQ){
      /*

      */
      //fprintf(listing, "VAR : %s, LINHA:%d, TIPO:%s \n", t->child[0]->attr.name,t->lineno,t->child[1]->attr.type);
		//aqui trata o chamado de função pra uma variável
            //fprintf(listing, "TIPO: %s \n", t->child[1]->attr.type);
            //atribuicao de variavel
			//variavel = t->child[0]->attr.name
			//valor = t->child[1]->attr.name
			//variavel devidamente declarada

			if (t->child[1]->attr.type != NULL){
				if (strcmp(t->child[1]->attr.type, "Integer") == 0) { //variavel é inteiro
					//fprintf(listing, "ATRIBUICAO DE VARIAVEL NOME: %s TIPO: INTEIRO\n",t->child[0]->attr.name);
				}
        else if (strcmp(t->child[1]->attr.type, "funcao") == 0){
          int x = st_lookup_func(t->child[1]->attr.name);
          if (x == -1){
            declError(t, "Variavel recebendo valor de função Void");
          }
        }

				else if (strcmp(t->child[1]->attr.type, "chamadaFuncao") == 0){ //variavel armazena uma funcao
					//fprintf(listing, "ATRIBUICAO DE VARIAVEL:%s = %s\n",t->child[0]->attr.name, t->child[1]->attr.name);
					if (st_lookup(t->child[1]->attr.name) == -1) {
						if((strcmp(t->child[1]->attr.name, "input")!=0))
						//chamando uma função que não foi declarada
						declError(t,"Variavel recebendo função que não foi declarada.");
					}
					else{
              st_add_lineno(t->child[1]->attr.name, t->lineno);
						//função foi declarada, armazena na tabela

					}
				}
			}

//			fprintf(listing, "ATRIBUICAO DE VARIAVEL %s\n",t->child[1]->attr.name);

		}


          break;
	case ConstK:
          break;
	case IdK:


            //declaraçao de variavel, tipo: t->attr.type nome: $t->attr.name
            //variavel pode ser do tipo id,
            if (strcmp(t->attr.type, "id") == 0) {

                if (cgen_search_top(t->attr.name) != -1) { //variavel ja existe, basta adicionar na tabela
					st_add_lineno(t->attr.name, t->lineno); //adiciona a variavel na tabela
				}
                else{
                    //erro, chamada de variavel que nao existe
                    declError(t, "Variavel não declarada.");
                }
            }
            else if (strcmp(t->attr.type, "chamadaFuncao") == 0){
                //funcao esta sendo chamada pela variavel
            }
            else if (strcmp(t->attr.type, "funcao") == 0){
                //é uma função
            }
            else if (strcmp(t->attr.type, "variavel") == 0){
                //é uma variavel
            }

          break;
	case IntK:
          break;
	case VoidK:
          break;
	case AtivK:
		//chamada de funcao
	if (busca_funcao_escopo_global(t->attr.name) == -1) {
		declError(t, "Chamada de função não declarada");
	} else {
		//aki
    //verificar se variavel é void
    //fprintf(listing, "VAR FUNC: %d\n",x);
    //fprintf(listing, "CHEGOU AKI %s, X: %d\n",t->attr.name,x);
		st_add_lineno(t->attr.name, t->lineno);
	}

          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}


static void afterInsertNode( TreeNode * t ) {
	switch (t->nodekind) {
		case StmtK:
			switch (t->kind.stmt) { /*aqui*/
				case FuncaoK:
		  			sc_pop();
		 			break;
	       			default:
		  			break;
	     		}
	      		break;
	    	default:
	      		break;
  	}
}

static void inputOutputInsert(void){
	TreeNode *input;

	input = newExpNode(FuncaoK);
	input->lineno = -1;
	input->attr.name = "input";
	input->attr.type = "Função";
	st_insert(input->attr.name,"Integer",input->lineno,addLocation(),input); //insere na tabela

	TreeNode *output;

	output = newExpNode(FuncaoK);
	output->lineno = -1;
	output->attr.name = "output";
	output->attr.type = "Função";
	st_insert(output->attr.name,"Void",output->lineno,addLocation(),output); //insere na tabela


}

/* Function buildSymtab constructs the symbol
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree) {
	globalScope = sc_create(NULL);
	sc_push(globalScope);
	inputOutputInsert();
	traverse(syntaxTree,insertNode,afterInsertNode);
	sc_pop();
	if(mainDeclarada == 0) {
		fprintf(listing, "Erro: Nao existe uma funcao main declarada.\n");
		Error = TRUE;
    		return;
	}

 	if (TraceAnalyze) {
		fprintf(listing,"\nTable of Symbols:\n\n");
    		printSymTab(listing);
  	}
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}


static void beforeCheckNode(TreeNode * t) {
	switch (t->nodekind) {
    		case StmtK:
			/*switch (t->kind.stmt) {
				case CompK:
          				sc_push(t->scope);
          				break;
        			default:
          				break;
      			}*/
		case ExpK:
			switch (t->kind.stmt) {
				case FuncaoK:
					nomeFuncao = t->attr.name;
					// sc_push(t->scope);
					break;
				default:
					break;
			}
    		default:
      			break;
  	}
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      {
	case OpK:
          break;
	case ConstK:
          break;
	case IdK:
          break;
	case IntK:
          break;
	case VoidK:
          break;
	case AtivK:
          break;
        default:
          break;
      }
      break;
    case StmtK:

      switch (t->kind.stmt)
      {
	case CompK:
	break;
	case FuncaoK:
	break;
	case VectorK:
	break;
	case VarK:
	break;
	case ReturnK:
	break;
	case AssignK:
	break;
	case WhileK:
	break;
	case IfK:
	break;
        default:
          break;
      }

/*
      switch (t->kind.stmt)
      { case IfK:
          if (t->child[0]->type == Integer)
            typeError(t->child[0],"if test is not Boolean");
          break;
        case AssignK:
          if (t->child[0]->type != Integer)
            typeError(t->child[0],"assignment of non-integer value");
          break;
        case WriteK:
          if (t->child[0]->type != Integer)
            typeError(t->child[0],"write of non-integer value");
          break;
        case RepeatK:
          if (t->child[1]->type == Integer)
            typeError(t->child[1],"repeat test is not Boolean");
          break;
        default:
          break;
      }
*/
      break;
    default:
      break;

  }
}

/* Procedure typeCheck performs type checking
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,nullProc,checkNode);
}
