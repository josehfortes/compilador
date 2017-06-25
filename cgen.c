/****************************************************/
/* File: cgen.c                                     */
/* The code generator implementation                */
/* for the TINY compiler                            */
/* (generates code for the TM machine)              */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"

/* tmpOffset is the memory offset for temps
   It is decremented each time a temp is
   stored, and incremeted when loaded again
*/
static int tmpOffset = 0;

//vaiavel globals
Operand OpGlobal;
int tempT = 0;
int tempL = 0;

/* Function ail_initialize initializes
 * the assembly instruction list
*/
void ail_initialize(){
    ail = NULL;
}

void asl_initialize(){
    asl = NULL;
}

/* Functions ail_create[1-2]? create
 * an instruction to be inserted in
 * the assembly instruction list using
 * an AsmInstKind and zero,
 * one or two Operands
*/

AssemblyList asl_create(Instruction ins, AssemblyOperand aso){
    AssemblyList as = (AssemblyList) malloc(sizeof(struct AssemblyList));
    as->ins = ins;
    as->op1 = aso;
    return as;
}

AsmInstList ail_create(AsmInstKind aik, Operand op1, Operand op2, Operand op3){
    AsmInstList ai = (AsmInstList) malloc(sizeof(struct AsmInstList));
    ai->aik = aik;
    ai->op1 = op1;
	ai->op2 = op2;
	ai->op3 = op3;
    return ai;
}

static int registradores[32];
static int temporarioRegistrador[32];

void alocaTemporarioReg(int ntemp){
	int reg = busca_reg_livre();
	temporarioRegistrador[ntemp] = reg;
}
void limpaTemporarioReg(int ntemp){
	limpa_reg(temporarioRegistrador[ntemp]);
}

int busca_reg_livre(){
	for(int i=0;i<32;i++){
		if(registradores[i] == 0){
			usa_reg(i);
			return i;
		}
	}
	return -1;
}

void usa_reg(int r){
	registradores[r] = 1;
}

void limpa_reg(int r){
  registradores[r] = 0;
}

void gera_assembly(){
  AsmInstList t = ail;
  int posmem1;
	int posmem2;
	int reg1 = 0;
	int reg2 = 0;
  int regsoma;
  while(t != NULL){
		switch(t->aik){
		}
    t = t->next;
  }

}

void print_assembly(){
  /*
    FLAGS:
    0 para imediato
    1 para reg
    2 para pos de memoria
  */
  printf("-------------------- PRINT ASSEMBLY ------------------\n");
  AssemblyList t = asl;
  while(t != NULL){
    switch(t->ins){
      case ADD:
        printf("ADD REG_%d REG%d + REG%d\n",t->op1.value,t->op1.value2,t->op1.value3);
      break;
      case SUB:
        printf("SUB REG_%d REG%d - REG%d\n",t->op1.value,t->op1.value2,t->op1.value3);
      break;
      case MULT:
        printf("MULT REG_%d REG%d * REG%d\n",t->op1.value,t->op1.value2,t->op1.value3);
      break;
      case DIV:
        printf("DIV REG_%d REG%d / REG%d\n",t->op1.value,t->op1.value2,t->op1.value3);
      break;
      case ADDI:
        printf("ADDI REG_%d, %d\n",t->op1.value, t->op1.value2);
      break;
      case STORE:
        printf("STORE %d, REG_%d\n", t->op1.value,t->op1.value2);
      break;
      case LW:
        printf("LW REG%d, MEMORY[REG_%d]\n", t->op1.value,t->op1.value2);
      break;
    }
    t = t->next;
  }

  printf("------------------------------------------------------\n");
}

void ail_print(){
	char str[100];
	char str2[100];
  AsmInstList t = ail;
  while(t != NULL){
    switch(t->aik){
		case GotoK:
			printf("(Got,L%d,_,_)\n",t->op1.value);
		break;
		case LabK:
			if(t->op1.kind == TempK){
				printf("(Lab,L%d,_,_)\n",t->op1.value);
			}
			else{
				printf("(Lab,%d,_,_)\n",t->op1.value);
			}
		break;
		case AsgK:
			sprintf(str, "%d", t->op1.value);
			sprintf(str2, "%d", t->op2.value);
			if(t->op1.kind == VecAsgK){
				sprintf(str, "%d[%d]", t->op1.value, t->op1.tam);
				if(t->op1.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op1.value, t->op1.tam);
				}
			}
			//agora definimos o op2
			//ele pode ser um TempK, um ImmK um SymtabK ou um VecK
			if(t->op2.kind == TempK)
				sprintf(str2, "t%d", t->op2.value);
			else if (t->op2.kind == ImmK)
				sprintf(str2, "(%d)", t->op2.value);
			
			else if (t->op2.kind == VecK){
				
				sprintf(str2, "%d[%d]", t->op2.value, t->op2.tam);
				if(t->op2.type == ImmK)
					sprintf(str2, "%d[(%d)]", t->op2.value, t->op2.tam);
			}
			
			printf("(Asg,%s,%s,_)\n",str,str2);
		break;
		case FunctionParameterK:
		break;
		case CmpEqK:
		break;
		case CmpNEqK:
		break;
		case CmpGK:
		break;
		case CmpGEqK:
		break;
		case CmpLK:
		break;
		case CmpLEqK:
		break;
		case FunctionReturnK:
		break;
		case If_FK:
			printf("(if_f,t%d,L%d,_)\n",t->op1.value,t->op2.value);
		break;
		case AddK:
			sprintf(str, "%d", t->op1.value);
			sprintf(str2, "%d", t->op2.value);
			//verificamos o que é o op1
			if(t->op1.kind == VecK){
				sprintf(str, "%d[%d]", t->op1.value, t->op1.tam);
				if(t->op1.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op1.value, t->op1.tam);
				}
			}
			else if(t->op1.kind == ImmK)
				sprintf(str, "(%d)", t->op1.value);
			else if(t->op1.kind == TempK)
				sprintf(str, "t%d", t->op1.value);
			
			//verificamos o que é o op2
			if(t->op2.kind == VecK){
				sprintf(str2, "%d[%d]", t->op2.value, t->op2.tam);
				if(t->op2.type == ImmK){
					sprintf(str2, "%d[(%d)]", t->op2.value, t->op2.tam);
				}
			}
			else if(t->op2.kind == ImmK)
				sprintf(str2, "(%d)", t->op2.value);
			else if(t->op2.kind == TempK)
				sprintf(str2, "t%d", t->op2.value);
			printf("(Add,t%d,%s,%s)\n",t->op3.value,str,str2);
		break;
		case SubK:
			sprintf(str, "%d", t->op1.value);
			sprintf(str2, "%d", t->op2.value);
			//verificamos o que é o op1
			if(t->op1.kind == VecK){
				sprintf(str, "%d[%d]", t->op1.value, t->op1.tam);
				if(t->op1.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op1.value, t->op1.tam);
				}
			}
			else if(t->op1.kind == ImmK)
				sprintf(str, "(%d)", t->op1.value);
			else if(t->op1.kind == TempK)
				sprintf(str, "t%d", t->op1.value);
			
			//verificamos o que é o op2
			if(t->op2.kind == VecK){
				sprintf(str2, "%d[%d]", t->op2.value, t->op2.tam);
				if(t->op2.type == ImmK){
					sprintf(str2, "%d[(%d)]", t->op2.value, t->op2.tam);
				}
			}
			else if(t->op2.kind == ImmK)
				sprintf(str2, "(%d)", t->op2.value);
			else if(t->op2.kind == TempK)
				sprintf(str2, "t%d", t->op2.value);
			printf("(Sub,t%d,%s,%s)\n",t->op3.value,str,str2);
		break;
		case MultK:
			sprintf(str, "%d", t->op1.value);
			sprintf(str2, "%d", t->op2.value);
			//verificamos o que é o op1
			if(t->op1.kind == VecK){
				sprintf(str, "%d[%d]", t->op1.value, t->op1.tam);
				if(t->op1.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op1.value, t->op1.tam);
				}
			}
			else if(t->op1.kind == ImmK)
				sprintf(str, "(%d)", t->op1.value);
			else if(t->op1.kind == TempK)
				sprintf(str, "t%d", t->op1.value);
			
			//verificamos o que é o op2
			if(t->op2.kind == VecK){
				sprintf(str2, "%d[%d]", t->op2.value, t->op2.tam);
				if(t->op2.type == ImmK){
					sprintf(str2, "%d[(%d)]", t->op2.value, t->op2.tam);
				}
			}
			else if(t->op2.kind == ImmK)
				sprintf(str2, "(%d)", t->op2.value);
			else if(t->op2.kind == TempK)
				sprintf(str2, "t%d", t->op2.value);
			printf("(Mult,t%d,%s,%s)\n",t->op3.value,str,str2);
		break;
		case DivK:
			sprintf(str, "%d", t->op1.value);
			sprintf(str2, "%d", t->op2.value);
			//verificamos o que é o op1
			if(t->op1.kind == VecK){
				sprintf(str, "%d[%d]", t->op1.value, t->op1.tam);
				if(t->op1.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op1.value, t->op1.tam);
				}
			}
			else if(t->op1.kind == ImmK)
				sprintf(str, "(%d)", t->op1.value);
			else if(t->op1.kind == TempK)
				sprintf(str, "t%d", t->op1.value);
			
			//verificamos o que é o op2
			if(t->op2.kind == VecK){
				sprintf(str2, "%d[%d]", t->op2.value, t->op2.tam);
				if(t->op2.type == ImmK){
					sprintf(str2, "%d[(%d)]", t->op2.value, t->op2.tam);
				}
			}
			else if(t->op2.kind == ImmK)
				sprintf(str2, "(%d)", t->op2.value);
			else if(t->op2.kind == TempK)
				sprintf(str2, "t%d", t->op2.value);
			printf("(Div,t%d,%s,%s)\n",t->op3.value,str,str2);
		break;
		case FunctionCallK:
		break;
		default:
		break;
    }

    t = t->next;
  }
}

/* Function ail_insert inserts
 * an assembly instruction
 * in the assembly instruction list
*/
void ail_insert(AsmInstList ai){
    if(ail == NULL){
        ail = ai;
        ail->next = NULL;
    }
    else{
        AsmInstList t = ail;
        while(t->next != NULL)
            t = t->next;
        t->next = ai;
        t->next->next = NULL;
    }
}

void asl_insert(AssemblyList as){
  if(asl == NULL){
      asl = as;
      asl->next = NULL;
  }
  else{
      AssemblyList t = asl;
      while(t->next != NULL)
          t = t->next;
      t->next = as;
      t->next->next = NULL;
  }
}


/* prototype for internal recursive code generator */
static void cGen (TreeNode * tree);

/* Procedure genStmt generates code at a statement node */
static void genStmt( TreeNode * tree)
{
  int pos;
  Operand opn;
  Operand op1;
  Operand op2;
  Operand op3;
  switch (tree->kind.stmt) {
	case VectorK:
		printf("entrou no VectorK\n");
	break;
	case ReturnK:
		printf("entrou no ReturnK\n");
	break;
	case AssignK:
		printf("entrou no AssignK\n");
	break;
    case CompK:
		cGen(tree->child[1]);
		if (strcmp(tree->child[1], "funcao") == 0){
			cGen(tree->child[1]);
			Operand op1 = {TempK, tempT};
			ail_insert(ail_create(FunctionCallK,op1,opn,opn)); //LabK para label
		}
    break;
    case VarParK:
		printf("entrou no VarParK\n");
    break;
	case WhileK:
		printf("entrou no WhileK\n");
		int labInicio;
 		//cria a label inicial
 		tempL++;
 		labInicio = tempL;
 		Operand op3 = {TempK, labInicio};
 		ail_insert(ail_create(LabK, op3, opn, opn)); //LabK para label
 		//chama a expressao
 		cGen(tree->child[0]);
 		//cria o IF_F para a label L
		int labL;
 		tempL++;
 		labL = tempL;
		op1.kind = SymtabK;
		op1.value = tempT;
		op2.value = tempL;
 		ail_insert(ail_create(If_FK, op1, op2, opn)); //LabK para label
 		//chama o statement
 		cGen(tree->child[1]);
 		//cria o goto pra label inicial
 		Operand op4 = {TempK, labInicio};
 		ail_insert(ail_create(GotoK, op4, opn, opn)); //LabK para label
 		//cria a label final L
 		Operand op5 = {TempK, labL};
 		ail_insert(ail_create(LabK, op5, opn, opn)); //LabK para label
	break;
    case IfK:
		printf("entrou no IfK\n");
		
		int label1;
 		int label2;
 		//chama a expressao
 		cGen(tree->child[0]);
 
 		//cria o IF_F para a label 1
 		tempL++;
 		label1 = tempL;
 		Operand op6 = {SymtabK,tempT,label1};
		op2.value = label1;
 		ail_insert(ail_create(If_FK, op6, op2, opn)); //LabK para label
 		//chama o statement
 		cGen(tree->child[1]);
 
 
 		if(tree->child[2] != NULL){
 			//else do if
 			//precisamos de colocar um goto Lx para o fim do if não entrar no else
 			//cria o goto para o fim do if
 			tempL++;
 			label2 = tempL;
 			Operand op4 = {TempK, label2};
 			ail_insert(ail_create(GotoK, op4, opn, opn)); //LabK para label
 
 
 			//cria a label para o else do if (if_f)
 			Operand op3 = {TempK, label1};
 			ail_insert(ail_create(LabK, op3, opn, opn));
 			//chama o else
 			cGen(tree->child[2]);
 			//cria a label do fim do if para o goto
 			Operand op5 = {TempK, label2};
 			ail_insert(ail_create(LabK, op5, opn, opn)); //LabK para label
 		}
 		else{
 			//nao existe else no if, apenas colocamos a label pós if
 			//cria a label Lx
 			Operand op3 = {TempK, label1};
 			ail_insert(ail_create(LabK, op3, opn, opn)); //LabK para label
 		}
    break;
    case VarK:
		printf("entrou no VarK\n");
	break;
    case FuncaoK:
		printf("entrou no FuncaoK\n");
        pos = cgen_search_top(tree->child[0]->attr.name);
        Operand op1 = {SymtabK, pos};
        ail_insert(ail_create(LabK,op1,opn,opn)); //LabK para label
		//chama o cgen novamente para os filhos da esquerda e direita
		cGen(tree->child[0]->child[1]);
	break;
    }
} /* genStmt */

static Operand opn;
static Operand op1;
static Operand op2;
static Operand op3;
  
/* Procedure genExp generates code at an expression node */
static void genExp( TreeNode * tree)
{
  AsmInstKind Op1;
  
  switch (tree->kind.exp) {
	case ConstK :
		printf("entrou no constK\n");
		OpGlobal.value = tree->attr.val;
    break; // ConstK
	case IntK:
		printf("entrou no IntK\n");
	break;
	case AtivK:
		printf("entrou no ativk");
		//precisamos buscar quantos parametros a funcao temp
		int qt = 0;
		TreeNode * t = tree->child[0];
		while(t != NULL){
		 if (t->attr.type == NULL){
			 cGen(t);
			 //parametro é o temporario
			 Operand op2 = {TempK, tempT};
		     ail_insert(ail_create(FunctionParameterK, op2, opn, opn));
		 }
		 else{
			 if(strcmp(t->attr.type,"Integer") == 0){
				Operand op2 = {ImmK, t->attr.val};
				ail_insert(ail_create(FunctionParameterK, op2, opn, opn));
			 }
			 else{
				Operand op2 = {SymtabK, cgen_search_top(t->attr.name)};
				ail_insert(ail_create(FunctionParameterK, op2, opn, opn));
			 }
		 }
		 qt = qt+1;
		 t = t->sibling;
		}
		int funcao = cgen_search_top(tree->attr.name);
		tempT++;

		Operand op1 = {TempK, funcao, qt, tempT};
		ail_insert(ail_create(FunctionParameterK, op1, opn, opn));
		
	break;
    case IdK :
		printf("entrou no IdK\n");
		OpGlobal.value = cgen_search_top(tree->attr.name);
    break; /* IdK */
	case OpK:
		printf("entrou no OpK\n");
		//todas as operações
		
		//verifica se é um assign
		if(tree->attr.op == EQ){//é um assign, iremos no final criar o asgk
		
			int var1 = cgen_search_top(tree->child[0]->attr.name);//buscando a variavel que vai receber o assign
			//pode ser variavel ou vetor
			op1.kind = VarAsgK;
			op1.value = var1;
			
			if(strcmp(tree->child[0]->attr.typeVar, "vector") == 0){
				int posvec = tree->child[0]->child[0]->attr.val;
				//a variavel é um vetor
				op1.kind = VecAsgK;
				op1.value = var1;
				op1.tam = posvec;
				op1.type = ImmK;
				//precisamos de saber agora o tamanho do vetor, e se é uma variavel ou um numero
				if(strcmp(tree->child[0]->child[0]->attr.type,"Integer")!=0){
					//é uma variavel
					posvec = cgen_search_top(tree->child[0]->child[0]->attr.name);
					//altera o op1 para este ao inves do de variavel
					op1.tam = posvec;
					op1.type = SymtabK;
				}
			}
			//a partir daqui, temos o operando 1, precisamos do segundo
			//vamos ver o que esta depois do = agora
			/*
			* ele pode ser um TempK, um ImmK um SymtabK ou um VecK
			*/
			
			if(tree->child[1]->attr.type == NULL){
				//é um opk
				cGen(tree->child[1]);
				op2.kind = TempK;
				op2.value = tempT;
			}
			else{
				
				//é uma variavel ou uma chamada de funcao ou um inteiro
				if(strcmp(tree->child[1]->attr.type,"Integer")==0){
					//é um inteiro
					op2.kind = ImmK;
					op2.value = tree->child[1]->attr.val;
				}
				else{
					//variavel ou chamada de funcao
					cGen(tree->child[1]);
					if(strcmp(tree->child[1]->attr.type,"id")==0){
						//é uma variavel
						//precisa verificar se é vetor ou inteiro
						op2.kind = SymtabK;
						op2.value = cgen_search_top(tree->child[1]->attr.name);
						if(strcmp(tree->child[1]->attr.typeVar, "vector") == 0){
							//é um vetor
							op2.kind = VecK;
							//o value é o mesmo, precisamos de armazenar as informacoes do content agora
							
							int posvec2 = tree->child[1]->child[0]->attr.val;
							op2.tam = posvec2;
							op2.type = ImmK;
							
							
							if(strcmp(tree->child[1]->child[0]->attr.type,"Integer")!=0){
								//o valor de dentro do [] do vetor e uma variavel
								int posvec2 = cgen_search_top(tree->child[1]->child[0]->attr.name);
								op2.tam = posvec2;
								op2.type = SymtabK;
							}
							
						}
					}
					else{
						//é uma chamada de funcao, devemos chama-la e armazenar dps o seu retorno
						//o retorno estará em um temporario
						cGen(tree->child[1]);
						op2.kind = TempK;
						op2.value = tempT;
					}
				}
			}
			//temos os 2 operandos, vamos add na lista o asgk
			ail_insert(ail_create(AsgK,op1,op2,opn));
		} //fim do EQ
		else if ((tree->attr.op == PLUS) || (tree->attr.op == MINUS) || (tree->attr.op == TIMES) || (tree->attr.op == OVER)){
			//é uma operação
			
			//tratando o primeiro valor da operaca
			cGen(tree->child[0]);
			//assumimos que os valores sao imediatos
			op1.kind = ImmK;
			op1.value = OpGlobal.value;
			//precisamos de saber o que é esse valor (pode ser vetor, variavel ou imediato)
			if(tree->child[0]->attr.type == NULL){
				//é um temporario
				op1.value = tempT;
				op1.kind = TempK;
			}
			else if (strcmp(tree->child[0]->attr.type,"id") == 0){
				//é uma variavel
				//precisamos ainda de verificar se é vetor ou variavel comum, vamos assumir que é uma comum
				op1.kind = SymtabK;
				if(strcmp(tree->child[0]->attr.typeVar, "vector") == 0){
					//é um vetor, devemos fazer todo o tratamento...
					op1.kind = VecK;
					int posvec = tree->child[0]->child[0]->attr.val;
					op1.tam = posvec;
					op1.type = ImmK;
					if(strcmp(tree->child[0]->child[0]->attr.type,"Integer")!=0){
						//o valor de dentro do [] do vetor e uma variavel
						int posvec = cgen_search_top(tree->child[0]->child[0]->attr.name);
						op1.tam = posvec;
						op1.type = SymtabK;
					}
				}
			}
			//terminamos para a variavel 1, fazemos o mesmo pra 2
			
			cGen(tree->child[1]);
			//assumimos que os valores sao imediatos
			op2.kind = ImmK;
			op2.value = OpGlobal.value;
			//precisamos de saber o que é esse valor (pode ser vetor, variavel ou imediato)
			if(tree->child[1]->attr.type == NULL){
				//é um temporario
				op2.value = tempT;
				op2.kind = TempK;
			}
			else if (strcmp(tree->child[1]->attr.type,"id") == 0){
				//é uma variavel
				//precisamos ainda de verificar se é vetor ou variavel comum, vamos assumir que é uma comum
				op2.kind = SymtabK;
				if(strcmp(tree->child[1]->attr.typeVar, "vector") == 0){
					//é um vetor, devemos fazer todo o tratamento...
					op2.kind = VecK;
					int posvec = tree->child[1]->child[0]->attr.val;
					op2.tam = posvec;
					op2.type = ImmK;
					if(strcmp(tree->child[1]->child[0]->attr.type,"Integer")!=0){
						//o valor de dentro do [] do vetor e uma variavel
						int posvec = cgen_search_top(tree->child[1]->child[0]->attr.name);
						op2.tam = posvec;
						op2.type = SymtabK;
					}
				}
			}
			
			//Op1 armazena qual é a operação
			if(tree->attr.op == PLUS){
				Op1 = AddK;
			}
			else if(tree->attr.op == MINUS){
				Op1 = SubK;
			}
			else if(tree->attr.op == TIMES){
				Op1 = MultK;
			}
			else if(tree->attr.op == OVER){
				Op1 = DivK;
			}
			//precisamos setar um temporario pra instrucao e armazena-lo, este vai no op3
			tempT++;
			op3.value = tempT;
			//criamos na lista
			ail_insert(ail_create(Op1,op1,op2,op3));
			
		}
		else{
			//comparacoes
			
		}
		
		
	break;
  }
} /* genExp */

/* Procedure cGen recursively generates code by
 * tree traversal
 */
static void cGen( TreeNode * tree)
{
  if (tree != NULL)
  {

    switch (tree->nodekind) {
      case StmtK:
        genStmt(tree);
        break;
      case ExpK:
        genExp(tree);
        break;
      default:
        break;
    }
    cGen(tree->sibling);
  }
}



/**********************************************/
/* the primary function of the code generator */
/**********************************************/
/* Procedure codeGen generates code to a code
 * file by traversal of the syntax tree. The
 * second parameter (codefile) is the file name
 * of the code file, and is used to print the
 * file name as a comment in the code file
 */
void codeGen(TreeNode * syntaxTree, char * codefile)
{

	//inicializando os registradores como 0, o reg0 é sempre 0, portanto, ocupado
	registradores[0] = 1;
	temporarioRegistrador[0] = 0;
	for(int i=1; i<32;i++){
		registradores[i] = 0;
		temporarioRegistrador[i] = 0;
	}


   ail_initialize();
   asl_initialize();

   char * s = malloc(strlen(codefile)+7);
   strcpy(s,"File: ");
   strcat(s,codefile);
   emitComment("TINY Compilation to TM Code");
   emitComment(s);
   /* generate standard prelude */
   emitComment("Standard prelude:");
   emitRM("LD",mp,0,ac,"load maxaddress from location 0");
   emitRM("ST",ac,0,ac,"clear location 0");
   emitComment("End of standard prelude.");
   /* generate code for TINY program */
   cGen(syntaxTree);
   /* finish */
   emitComment("End of execution.");
   emitRO("HALT",0,0,0,"");

   ail_print();
   gera_assembly();
   print_assembly();

}