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
		break;
		case AddK:
		break;
		case SubK:
		break;
		case MultK:
		break;
		case DivK:
		break;
		case FunctionCallK:
		break;
		case VarAsgK:
		break;
		case VecAsgK:
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
	break;
	case ReturnK:
	break;
	case AssignK:
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
    break;
	case WhileK:
	break;
    case IfK:
    break;
    case VarK:
	break;
    case FuncaoK:
        pos = cgen_search_top(tree->child[0]->attr.name);
        Operand op1 = {SymtabK, pos};
        ail_insert(ail_create(LabK,op1,opn,opn)); //LabK para label
		//chama o cgen novamente para os filhos da esquerda e direita
		cGen(tree->child[0]->child[1]);
	break;
    }
} /* genStmt */

/* Procedure genExp generates code at an expression node */
static void genExp( TreeNode * tree)
{
  Operand opn;
  Operand op1;
  Operand op2;
  Operand op3;
  AsmInstKind Op1;
  
  switch (tree->kind.exp) {
	case ConstK :
		OpGlobal.value = tree->attr.val;
    break; // ConstK
	case IntK:
	break;
	case AtivK:
		printf("");
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
		OpGlobal.value = cgen_search_top(tree->attr.name);
    break; /* IdK */
	case OpK:
		//todas as operações
		//Op1 armazena se é alguma operação
		if(tree->attr.op == PLUS){
			printf("É UM ADD\n");
			Op1 = AddK;
		}
		else if(tree->attr.op == MINUS){
			Op1 = AddK;
		}
		else if(tree->attr.op == TIMES){
			Op1 = AddK;
		}
		else if(tree->attr.op == OVER){
			Op1 = AddK;
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
