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

AsmInstList ail_create(AsmInstKind aik){
    AsmInstList ai = (AsmInstList) malloc(sizeof(struct AsmInstList));
    ai->aik = aik;
    return ai;
}

AsmInstList ail_create1(AsmInstKind aik, Operand op1){
    AsmInstList ai = (AsmInstList) malloc(sizeof(struct AsmInstList));
    ai->aik = aik;
    ai->op1 = op1;
    return ai;
}

AsmInstList ail_create2(AsmInstKind aik, Operand op1, Operand op2){
    AsmInstList ai = (AsmInstList) malloc(sizeof(struct AsmInstList));
    ai->aik = aik;
    ai->op1 = op1;
    ai->op2 = op2;
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
	//printf("SUB REG_%d REG_%d REG_%d\n", r, r, r);
  AssemblyOperand op = {r,1,r,1,r,1};
  asl_insert(asl_create(SUB, op));
	registradores[r] = 0;
}

int getVarFromMemory(int pos){
	//temos a posicao has pos
	//buscamos a posicao na memoria dessa variavel
	int posmem = search_pos_var (pos);
	//buscamos um registrador livre
	int reg = busca_reg_livre();
	//posmem é a posicao de memoria, devmeos usar um registrador para ler a memoria
	int reg1 = busca_reg_livre();
	
    //usamos o addi
    AssemblyOperand op1 = {reg1,1,decimal_binario(posmem),0};
    asl_insert(asl_create(ADDI, op1));
	//efetuamos um LW somando com o REG_0 e
	AssemblyOperand op2 = {reg,1,reg1,1};
	asl_insert(asl_create(LW, op2));
	//printf("LW REG_%d, MEMORY[REG_0 + %d]\n", reg, posmem);
	limpa_reg(reg1);
	return reg;
	//pronto, o reg possui o valor que estava na memoria agr
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
      case AddK:
        //tempvalue pode assumir 2 valores:
        /*
        -1 para variavel
        0 para imediato
        1 para temporário (registrador)
        */
        //verificamos se o valor 1 ja é um registrador, se sim, apenas prosseguimos
        //se não, devemos buscar o valor
        if(t->op1.tempValue == 1){
          //é um temporario, pegamos o seu resultado e armazenamos no reg1
          //no fim, desalocamos esse registrador
          reg1 = temporarioRegistrador[t->op1.value];
        }
        else{
          //precisamos de buscar a variavel, agora devemos saber se é um imediato ou uma variavel
          if(t->op1.tempValue == -1){
            //é uma variavel
            //devemos buscar essa variavel na memoria
            //passamos como parametro a pos hash dela
            reg1 = getVarFromMemory(t->op1.value);
            //agora temos o valor no reg1
          }
          else{
            //é um imediato
            //precisamos de passar o valor pra um registrador
            //alocamos um registrador pra essa operacao
            reg1 = busca_reg_livre();
            //usamos o addi

            AssemblyOperand op1 = {reg1,1,decimal_binario(t->op1.value),0};
            asl_insert(asl_create(ADDI, op1));
            //printf("ADDI REG_%d, %d\n", reg1, decimal_binario(t->op1.value));

          }
        }

        //verificamos se o valor 2 ja é um registrador, se sim, apenas prosseguimos
        //se não, devemos buscar o valor
        if(t->op1.tempValue2 == 1){
                    //é um temporario, pegamos o seu resultado e armazenamos no reg1
          //no fim, desalocamos esse registrador
          reg2 = temporarioRegistrador[t->op1.value2];
        }
        else{
          //precisamos de buscar a variavel, agora devemos saber se é um imediato ou uma variavel
          if(t->op1.tempValue2 == -1){
            //é uma variavel
            //devemos buscar essa variavel na memoria
            //passamos como parametro a pos hash dela
            reg2 = getVarFromMemory(t->op1.value2);
            //agora temos o valor no reg1
          }
          else{
            //é um imediato
            //precisamos de passar o valor pra um registrador
            //alocamos um registrador pra essa operacao
            reg2 = busca_reg_livre();
            //usamos o addi
            AssemblyOperand op1 = {reg2,1,decimal_binario(t->op1.value2),0};
            asl_insert(asl_create(ADDI, op1));
            //printf("ADDI REG_%d, %d\n", reg2, decimal_binario(t->op1.value2));
          }
        }
        //agora devemos somar os 2 registradores e armazenar em um terceiro
        //esse registrador ja esta alocado pois é um temporario, devemos busca-lo

        regsoma = temporarioRegistrador[t->op1.pos];

        AssemblyOperand op1 = {regsoma,1,reg1,1,reg2,1};
        asl_insert(asl_create(ADD, op1));
        //printf("ADD REG_%d, REG_%d + REG_%d\n",regsoma,reg1,reg2);
        //limpamos os dois regs
        limpa_reg(reg1);
        limpa_reg(reg2);
      break;
      case  SubK:
				//tempvalue pode assumir 2 valores:
				/*
				-1 para variavel
				0 para imediato
				1 para temporário (registrador)
				*/
				//verificamos se o valor 1 ja é um registrador, se sim, apenas prosseguimos
				//se não, devemos buscar o valor
				if(t->op1.tempValue == 1){
					//é um temporario, pegamos o seu resultado e armazenamos no reg1
					//no fim, desalocamos esse registrador
					reg1 = temporarioRegistrador[t->op1.value];
				}
				else{
					//precisamos de buscar a variavel, agora devemos saber se é um imediato ou uma variavel
					if(t->op1.tempValue == -1){
						//é uma variavel
						//devemos buscar essa variavel na memoria
						//passamos como parametro a pos hash dela
						reg1 = getVarFromMemory(t->op1.value);
						//agora temos o valor no reg1
					}
					else{
						//é um imediato
						//precisamos de passar o valor pra um registrador
						//alocamos um registrador pra essa operacao
						reg1 = busca_reg_livre();
						//usamos o addi
            AssemblyOperand op1 = {reg1,1,decimal_binario(t->op1.value),0};
            asl_insert(asl_create(ADDI, op1));
						//printf("ADDI REG_%d, %d\n", reg1, decimal_binario(t->op1.value));

					}
				}

				//verificamos se o valor 2 ja é um registrador, se sim, apenas prosseguimos
				//se não, devemos buscar o valor
				if(t->op1.tempValue2 == 1){
										//é um temporario, pegamos o seu resultado e armazenamos no reg1
					//no fim, desalocamos esse registrador
					reg2 = temporarioRegistrador[t->op1.value2];
				}
				else{
					//precisamos de buscar a variavel, agora devemos saber se é um imediato ou uma variavel
					if(t->op1.tempValue2 == -1){
						//é uma variavel
						//devemos buscar essa variavel na memoria
						//passamos como parametro a pos hash dela
						reg2 = getVarFromMemory(t->op1.value2);
						//agora temos o valor no reg1
					}
					else{
						//é um imediato
						//precisamos de passar o valor pra um registrador
						//alocamos um registrador pra essa operacao
						reg2 = busca_reg_livre();
						//usamos o addi
            AssemblyOperand op1 = {reg2,1,decimal_binario(t->op1.value2),0};
            asl_insert(asl_create(ADDI, op1));
						//printf("ADDI REG_%d, %d\n", reg2, decimal_binario(t->op1.value2));
					}
				}
				//agora devemos somar os 2 registradores e armazenar em um terceiro
				//esse registrador ja esta alocado pois é um temporario, devemos busca-lo
				regsoma = temporarioRegistrador[t->op1.pos];
        AssemblyOperand op2 = {regsoma,1,reg1,1,reg2,1};
        asl_insert(asl_create(SUB, op2));
				//printf("SUB REG_%d - REG_%d = REG_%d\n",reg1,reg2, regsoma);
				//limpamos os dois regs
				limpa_reg(reg1);
				limpa_reg(reg2);
			break;

      case  MultK:
				//tempvalue pode assumir 2 valores:
				/*
				-1 para variavel
				0 para imediato
				1 para temporário (registrador)
				*/
				//verificamos se o valor 1 ja é um registrador, se sim, apenas prosseguimos
				//se não, devemos buscar o valor
				if(t->op1.tempValue == 1){
					//é um temporario, pegamos o seu resultado e armazenamos no reg1
					//no fim, desalocamos esse registrador
					reg1 = temporarioRegistrador[t->op1.value];
				}
				else{
					//precisamos de buscar a variavel, agora devemos saber se é um imediato ou uma variavel
					if(t->op1.tempValue == -1){
						//é uma variavel
						//devemos buscar essa variavel na memoria
						//passamos como parametro a pos hash dela
						reg1 = getVarFromMemory(t->op1.value);
						//agora temos o valor no reg1
					}
					else{
						//é um imediato
						//precisamos de passar o valor pra um registrador
						//alocamos um registrador pra essa operacao
						reg1 = busca_reg_livre();
						//usamos o addi

            AssemblyOperand op1 = {regsoma,1,reg1,1,reg2,1};
            asl_insert(asl_create(ADDI, op1));
						//printf("ADDI REG_%d, %d\n", reg1, decimal_binario(t->op1.value));

					}
				}

				//verificamos se o valor 2 ja é um registrador, se sim, apenas prosseguimos
				//se não, devemos buscar o valor
				if(t->op1.tempValue2 == 1){
										//é um temporario, pegamos o seu resultado e armazenamos no reg1
					//no fim, desalocamos esse registrador
					reg2 = temporarioRegistrador[t->op1.value2];
				}
				else{
					//precisamos de buscar a variavel, agora devemos saber se é um imediato ou uma variavel
					if(t->op1.tempValue2 == -1){
						//é uma variavel
						//devemos buscar essa variavel na memoria
						//passamos como parametro a pos hash dela
						reg2 = getVarFromMemory(t->op1.value2);
						//agora temos o valor no reg1
					}
					else{
						//é um imediato
						//precisamos de passar o valor pra um registrador
						//alocamos um registrador pra essa operacao
						reg2 = busca_reg_livre();
						//usamos o addi
            AssemblyOperand op1 = {reg2,1,decimal_binario(t->op1.value2),0};
            asl_insert(asl_create(ADDI, op1));
            //printf("ADDI REG_%d, %d\n", reg2, decimal_binario(t->op1.value2));
					}
				}
				//agora devemos somar os 2 registradores e armazenar em um terceiro
				//esse registrador ja esta alocado pois é um temporario, devemos busca-lo

				regsoma = temporarioRegistrador[t->op1.pos];
        AssemblyOperand op3 = {regsoma,1,reg1,1,reg2,1};
        asl_insert(asl_create(MULT, op3));
				//printf("MULT REG_%d, REG_%d * REG_%d\n",regsoma,reg1,reg2);
				//limpamos os dois regs
				limpa_reg(reg1);
				limpa_reg(reg2);
			break;



      case  DivK:
				//tempvalue pode assumir 2 valores:
				/*
				-1 para variavel
				0 para imediato
				1 para temporário (registrador)
				*/
				//verificamos se o valor 1 ja é um registrador, se sim, apenas prosseguimos
				//se não, devemos buscar o valor
				if(t->op1.tempValue == 1){
					//é um temporario, pegamos o seu resultado e armazenamos no reg1
					//no fim, desalocamos esse registrador
					reg1 = temporarioRegistrador[t->op1.value];
				}
				else{
					//precisamos de buscar a variavel, agora devemos saber se é um imediato ou uma variavel
					if(t->op1.tempValue == -1){
						//é uma variavel
						//devemos buscar essa variavel na memoria
						//passamos como parametro a pos hash dela
						reg1 = getVarFromMemory(t->op1.value);
						//agora temos o valor no reg1
					}
					else{
						//é um imediato
						//precisamos de passar o valor pra um registrador
						//alocamos um registrador pra essa operacao
						reg1 = busca_reg_livre();
						//usamos o addi
            AssemblyOperand op1 = {reg1,1,decimal_binario(t->op1.value),0};
            asl_insert(asl_create(ADDI, op1));
						//printf("ADDI REG_%d, %d\n", reg1, decimal_binario(t->op1.value));

					}
				}

				//verificamos se o valor 2 ja é um registrador, se sim, apenas prosseguimos
				//se não, devemos buscar o valor
				if(t->op1.tempValue2 == 1){
										//é um temporario, pegamos o seu resultado e armazenamos no reg1
					//no fim, desalocamos esse registrador
					reg2 = temporarioRegistrador[t->op1.value2];
				}
				else{
					//precisamos de buscar a variavel, agora devemos saber se é um imediato ou uma variavel
					if(t->op1.tempValue2 == -1){
						//é uma variavel
						//devemos buscar essa variavel na memoria
						//passamos como parametro a pos hash dela
						reg2 = getVarFromMemory(t->op1.value2);
						//agora temos o valor no reg1
					}
					else{
						//é um imediato
						//precisamos de passar o valor pra um registrador
						//alocamos um registrador pra essa operacao
						reg2 = busca_reg_livre();
						//usamos o addi
            AssemblyOperand op1 = {reg2,1,decimal_binario(t->op1.value2),0};
            asl_insert(asl_create(ADDI, op1));
						//printf("ADDI REG_%d, %d\n", reg2, decimal_binario(t->op1.value2));
					}
				}
				//agora devemos somar os 2 registradores e armazenar em um terceiro
				//esse registrador ja esta alocado pois é um temporario, devemos busca-lo

				regsoma = temporarioRegistrador[t->op1.pos];
        AssemblyOperand op4 = {regsoma,1,reg1,1,reg2,1};
        asl_insert(asl_create(DIV, op4));
				//printf("DIV REG_%d, REG_%d/REG_%d\n",regsoma,reg1,reg2);
				//limpamos os dois regs
				limpa_reg(reg1);
				limpa_reg(reg2);
			break;


      case VarAsgK:
				//buscar a posicao no menloc da variavel - variavel t->op1.value
				posmem1 = search_pos_var (t->op1.value); //posicao de memoria que encontra-se a variavel q vai receber o dado

				//preciso de saber se o dado que será escrito é outra variável ou uma cte
				if(t->op1.kind == TempK){ //um opk (temporario)
					//o valor ja esta no registrador, devemos apenas dar um store com esse reg
					//buscamos o valor do registrador
					int reg = temporarioRegistrador[t->op1.pos];
					//chamamos o store
					  AssemblyOperand op1 = {posmem1,1,reg,1};
					  asl_insert(asl_create(STORE, op1));
					//printf("STORE %d, REG_%d\n", posmem1, reg);
					//printf("(asg,%d,t%d,_)\n",t->op1.value,t->op1.pos);
					limpa_reg(reg);

				}
				else if(t->op1.kind == ImmK){ //um inteiro
					//printf("(asg,%d,(%d),_)\n",t->op1.value,t->op1.pos);
					//já possuimos o valor, precisamos de armazenar em um registrador
					int reg = busca_reg_livre();//registrador livre pra armazenar
					//fazemos um ADDI pra adicionar o valor ao registrador
          //usamos o addi
          AssemblyOperand op1 = {reg,1,decimal_binario(t->op1.pos),0};
          asl_insert(asl_create(ADDI, op1));
          //printf("ADDI REG_%d, %d\n", reg, decimal_binario(t->op1.pos));
					//armazenamos na memoria o valor desse registrador
          AssemblyOperand op2 = {posmem1,2,reg,1};
          asl_insert(asl_create(STORE, op2));
          //printf("STORE %d, REG_%d\n", posmem1, reg);
					//resetamos o registrador
					limpa_reg(reg);

				}
				else if(t->op1.kind == VecK){
					//um vetor

					//precisamos buscar o valor da variavel na memoria
					posmem2 = search_pos_var (t->op1.pos);
					//precisamos de somar a posicao
					posmem2 += t->op1.value2;
					//precisamos diminuir 1 pois o vetor começa em 0
					posmem2--;
					int reg = busca_reg_livre();//registrador livre pra armazenar a segunda variavel
					//carregamos o valor da variavel nesse registrador reg2
          AssemblyOperand op2 = {reg,1,0,1,posmem2,2};
          asl_insert(asl_create(LW, op2));
          //printf("LW REG_%d, MEMORY[REG_0 + %d]\n", reg, posmem2);
					//armazenamos na memoria o valor desse registrador
          AssemblyOperand op3 = {posmem1,2,reg,1};
          asl_insert(asl_create(STORE, op2));
          //printf("STORE %d, REG_%d\n", posmem1, reg);
					//resetamos o registrador
					limpa_reg(reg);
					//printf("(asg,%d,%d[%d],_)\n",t->op1.value,t->op1.pos, t->op1.value2);
				}
				else{ //uma variavel inteiro
					//precisamos buscar o valor da variavel na memoria
					posmem2 = search_pos_var (t->op1.pos);
					int reg = busca_reg_livre();//registrador livre pra armazenar a segunda variavel
					//carregamos o valor da variavel nesse registrador reg2
          AssemblyOperand op2 = {reg,1,0,1,posmem2,2};
          asl_insert(asl_create(LW, op2));
          //printf("LW REG_%d, MEMORY[REG_0 + %d]\n", reg, posmem2);
					//armazenamos na memoria o valor desse registrador
          AssemblyOperand op3 = {posmem1,2,reg,1};
          asl_insert(asl_create(STORE, op2));
          //printf("STORE %d, REG_%d\n", posmem1, reg);
					//resetamos o registrador
					limpa_reg(reg);
					//printf("(asg,%d,%d,_)\n",t->op1.value,t->op1.pos);
				}
				//printf("Var: %d,Menloc: %d",t->op1.value, posmem1);
				//b
			break;

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
		int tempVar1;
		int tempVar2;
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
		if(t->op1.kind == TempK){ //um opk (temporario)
			printf("(par,t%d,_,_)\n",t->op1.value);
		}
		else if(t->op1.kind == ImmK){ //um inteiro
			printf("(par,(%d),_,_)\n",t->op1.value);
		}
		else{
			printf("(par,%d,_,_)\n",t->op1.value);
		}
	  break;
	  case CmpEqK:
		if(t->op1.kind == TempK){ //um opk (temporario)
			printf("(eq,%d,t%d,t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else if(t->op1.kind == ImmK){ //um inteiro
			printf("(eq,%d,(%d),t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else if (t->op1.kind == VarK){//uma variavel
			printf("(eq,%d,%d,t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else{ //vetor
			printf("(eq,%d,%d[%d],t%d)\n",t->op1.value,t->op1.value2, t->op1.pos,t->op1.tempValue);
		}
	  break;
	  case CmpNEqK:
		if(t->op1.kind == TempK){ //um opk (temporario)
			printf("(neq,%d,t%d,t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else if(t->op1.kind == ImmK){ //um inteiro
			printf("(neq,%d,(%d),t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else if (t->op1.kind == VarK){//uma variavel
			printf("(neq,%d,%d,t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else{ //vetor
			printf("(neq,%d,%d[%d],t%d)\n",t->op1.value,t->op1.value2, t->op1.pos,t->op1.tempValue);
		}
	  break;
	  case CmpGK:
		if(t->op1.kind == TempK){ //um opk (temporario)
			printf("(gt,%d,t%d,t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else if(t->op1.kind == ImmK){ //um inteiro
			printf("(gt,%d,(%d),t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else if (t->op1.kind == VarK){//uma variavel
			printf("(gt,%d,%d,t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else{ //vetor
			printf("(gt,%d,%d[%d],t%d)\n",t->op1.value,t->op1.value2, t->op1.pos,t->op1.tempValue);
		}
	  break;
	  case CmpGEqK:
		if(t->op1.kind == TempK){ //um opk (temporario)
			printf("(get,%d,t%d,t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else if(t->op1.kind == ImmK){ //um inteiro
			printf("(get,%d,(%d),t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else if (t->op1.kind == VarK){//uma variavel
			printf("(get,%d,%d,t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else{ //vetor
			printf("(get,%d,%d[%d],t%d)\n",t->op1.value,t->op1.value2, t->op1.pos,t->op1.tempValue);
		}
	  break;
	  case CmpLK:
		if(t->op1.kind == TempK){ //um opk (temporario)
			printf("(lt,%d,t%d,t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else if(t->op1.kind == ImmK){ //um inteiro
			printf("(lt,%d,(%d),t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else if (t->op1.kind == VarK){//uma variavel
			printf("(lt,%d,%d,t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else{ //vetor
			printf("(lt,%d,%d[%d],t%d)\n",t->op1.value,t->op1.value2, t->op1.pos,t->op1.tempValue);
		}
	  break;
	  case CmpLEqK:
		if(t->op1.kind == TempK){ //um opk (temporario)
			printf("(let,%d,t%d,t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else if(t->op1.kind == ImmK){ //um inteiro
			printf("(let,%d,(%d),t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else if (t->op1.kind == VarK){//uma variavel
			printf("(let,%d,%d,t%d)\n",t->op1.value,t->op1.value2, t->op1.pos);
		}
		else{ //vetor
			printf("(let,%d,%d[%d],t%d)\n",t->op1.value,t->op1.value2, t->op1.pos,t->op1.tempValue);
		}
	  break;
	  case FunctionReturnK:
		if(t->op1.kind == ImmK){
			printf("(ret,(%d),_,_)\n",t->op1.value);
		}
		else if(t->op1.kind == SymtabK){
			printf("(ret,%d,_,_)\n",t->op1.value);
		}
		else if(t->op1.kind == TempK){
			printf("(ret,t%d,_,_)\n",t->op1.value);
		}
	  break;
	  case If_FK:
		printf("(if_f,t%d,L%d,_)\n",t->op1.value,t->op1.value2);
	  break;

	  case AddK:
		//tempvalue pode assumir 2 valores:
		/*
		-1 para variavel
		0 para imediato
		1 para temporário (registrador)
		*/
		//verificamos primeiro se os dois sao iguais e de um tipo especifico
		sprintf(str, "(%d)", t->op1.value);
		sprintf(str2, "(%d)", t->op1.value2);
		if(t->op1.tempValue == -1){
			sprintf(str, "%d", t->op1.value);
		}
		else if(t->op1.tempValue == 1){
			sprintf(str, "t%d", t->op1.value);
		}
		if(t->op1.tempValue2 == -1){
			sprintf(str2, "%d", t->op1.value2);
		}
		else if(t->op1.tempValue2 == 1){
			sprintf(str2, "t%d", t->op1.value2);
		}
		//devemos alocar um registrador e salvar na memoria qual registrador corresponde esse temporario do resultado
		alocaTemporarioReg(t->op1.pos);
		printf("(sum,%s,%s,t%d)\n",str,str2,t->op1.pos);
	  break;
	  case SubK:
    //tempvalue pode assumir 2 valores:
		/*
		-1 para variavel
		0 para imediato
		1 para temporário (registrador)
		*/
		//verificamos primeiro se os dois sao iguais e de um tipo especifico
		sprintf(str, "(%d)", t->op1.value);
		sprintf(str2, "(%d)", t->op1.value2);
		if(t->op1.tempValue == -1){
			sprintf(str, "%d", t->op1.value);
		}
		else if(t->op1.tempValue == 1){
			sprintf(str, "t%d", t->op1.value);
		}
		if(t->op1.tempValue2 == -1){
			sprintf(str2, "%d", t->op1.value2);
		}
		else if(t->op1.tempValue2 == 1){
			sprintf(str2, "t%d", t->op1.value2);
		}
		//devemos alocar um registrador e salvar na memoria qual registrador corresponde esse temporario do resultado
		alocaTemporarioReg(t->op1.pos);
		printf("(sub,%s,%s,t%d)\n",str,str2,t->op1.pos);
	  break;
	  case MultK:
    //tempvalue pode assumir 2 valores:
		/*
		-1 para variavel
		0 para imediato
		1 para temporário (registrador)
		*/
		//verificamos primeiro se os dois sao iguais e de um tipo especifico
		sprintf(str, "(%d)", t->op1.value);
		sprintf(str2, "(%d)", t->op1.value2);
		if(t->op1.tempValue == -1){
			sprintf(str, "%d", t->op1.value);
		}
		else if(t->op1.tempValue == 1){
			sprintf(str, "t%d", t->op1.value);
		}
		if(t->op1.tempValue2 == -1){
			sprintf(str2, "%d", t->op1.value2);
		}
		else if(t->op1.tempValue2 == 1){
			sprintf(str2, "t%d", t->op1.value2);
		}
		//devemos alocar um registrador e salvar na memoria qual registrador corresponde esse temporario do resultado
		alocaTemporarioReg(t->op1.pos);
		printf("(mult,%s,%s,t%d)\n",str,str2,t->op1.pos);

	  break;
	  case DivK:

		//tempvalue pode assumir 2 valores:
		/*
		-1 para variavel
		0 para imediato
		1 para temporário (registrador)
		*/
		//verificamos primeiro se os dois sao iguais e de um tipo especifico
		sprintf(str, "(%d)", t->op1.value);
		sprintf(str2, "(%d)", t->op1.value2);
		if(t->op1.tempValue == -1){
			sprintf(str, "%d", t->op1.value);
		}
		else if(t->op1.tempValue == 1){
			sprintf(str, "t%d", t->op1.value);
		}
		if(t->op1.tempValue2 == -1){
			sprintf(str2, "%d", t->op1.value2);
		}
		else if(t->op1.tempValue2 == 1){
			sprintf(str2, "t%d", t->op1.value2);
		}
		//devemos alocar um registrador e salvar na memoria qual registrador corresponde esse temporario do resultado
		alocaTemporarioReg(t->op1.pos);
		printf("(div,%s,%s,t%d)\n",str,str2,t->op1.pos);
	  break;
	  case FunctionCallK:
		if(t->op1.kind == TempK){
			printf("(cal,%d,%d,t%d)\n",t->op1.value,t->op1.value2,t->op1.pos);
		}

	  break;
	  case VarAsgK:
		if(t->op1.kind == TempK){ //um opk (temporario)
			printf("(asg,%d,t%d,_)\n",t->op1.value,t->op1.pos);
		}
		else if(t->op1.kind == ImmK){ //um inteiro
			printf("(asg,%d,(%d),_)\n",t->op1.value,t->op1.pos);
		}
		else if(t->op1.kind == VecK){
			//um vetor
			if(t->op1.vecType == SymtabK){ //é uma variavel
				printf("(asg,%d,%d[%d],_)\n",t->op1.value,t->op1.pos, t->op1.value2);
			}
			else{
				printf("(asg,%d,%d[(%d)],_)\n",t->op1.value,t->op1.pos, t->op1.value2);
			}
			
		}
		else{ //uma variavel inteiro
			printf("(asg,%d,%d,_)\n",t->op1.value,t->op1.pos);
		}
		break;
	  case VecAsgK:
		if(t->op1.kind == TempK){ //um opk (temporario)
			printf("(asg,%d[],t%d,_)\n",t->op1.value,t->op1.pos);
		}
		else if(t->op1.kind == ImmK){ //um inteiro
			printf("(asg,%d[],(%d),_)\n",t->op1.value,t->op1.pos);
		}
		else if(t->op1.kind == VecK){
			//um vetor
			printf("(asg,%d[],%d[%d],_)\n",t->op1.value,t->op1.pos, t->op1.value2);
		}
		else{ //uma variavel inteiro
			printf("(asg,%d[],%d,_)\n",t->op1.value,t->op1.pos);
		}
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
{ TreeNode * p1, * p2, * p3;
  int savedLoc1,savedLoc2,currentLoc;
  int loc;

  switch (tree->kind.stmt) {


	case VectorK:
	printf("");
	break;
	case ReturnK:
		printf("");
		if(strcmp(tree->child[0]->attr.type, "Integer") == 0){
			Operand op2 = {ImmK, tree->child[0]->attr.val};
			ail_insert(ail_create1(FunctionReturnK, op2)); //FunctionReturnK para retorno de inteiro
		}
		else if(strcmp(tree->child[0]->attr.type, "id") == 0){
			Operand op2 = {SymtabK, cgen_search_top(tree->child[0]->attr.name)};
			ail_insert(ail_create1(FunctionReturnK, op2)); //FunctionReturnK para retorno de variavel

		}
		else if(strcmp(tree->child[0]->attr.type, "funcao") == 0){

			//é necessario acessar o ativk
			cGen(tree->child[0]);
			Operand op2 = {TempK, tempT};
			ail_insert(ail_create1(FunctionReturnK, op2)); //FunctionReturnK para retorno de inteiro

		}
	break;
	case AssignK:
	printf("");
	break;


      case CompK:
        //printf("ENTROU NO COMPK\n");
		cGen(tree->child[1]);

		if (strcmp(tree->child[1], "funcao") == 0){
			cGen(tree->child[1]);
			Operand op4 = {TempK, tempT};
			ail_insert(ail_create1(FunctionCallK, op4)); //LabK para label
		}
      break;

      case VarParK:
      //declaracao de variavel nos parametros de uma funcao
        printf("ENTROU NO VARPARK\n");

      break;

	  case WhileK:

		printf("");
		int labInicio;
		//cria a label inicial
		tempL++;
		labInicio = tempL;
		Operand op3 = {TempK, labInicio};
		ail_insert(ail_create1(LabK, op3)); //LabK para label
		//chama a expressao
		cGen(tree->child[0]);
		//cria o IF_F para a label L
		int labL;
		tempL++;
		labL = tempL;
		Operand op1 = {SymtabK,tempT,tempL};
		ail_insert(ail_create1(If_FK, op1)); //LabK para label
		//chama o statement
		cGen(tree->child[1]);
		//cria o goto pra label inicial
		Operand op4 = {TempK, labInicio};
		ail_insert(ail_create1(GotoK, op4)); //LabK para label
		//cria a label final L
		Operand op5 = {TempK, labL};
		ail_insert(ail_create1(LabK, op5)); //LabK para label
	  break;

      case IfK:
        printf("");
		int label1;
		int label2;
		//chama a expressao
		cGen(tree->child[0]);

		//cria o IF_F para a label 1
		tempL++;
		label1 = tempL;
		Operand op6 = {SymtabK,tempT,label1};
		ail_insert(ail_create1(If_FK, op6)); //LabK para label
		//chama o statement
		cGen(tree->child[1]);


		if(tree->child[2] != NULL){
			//else do if
			//precisamos de colocar um goto Lx para o fim do if não entrar no else
			//cria o goto para o fim do if
			tempL++;
			label2 = tempL;
			Operand op4 = {TempK, label2};
			ail_insert(ail_create1(GotoK, op4)); //LabK para label


			//cria a label para o else do if (if_f)
			Operand op3 = {TempK, label1};
			ail_insert(ail_create1(LabK, op3));
			//chama o else
			cGen(tree->child[2]);
			//cria a label do fim do if para o goto
			Operand op5 = {TempK, label2};
			ail_insert(ail_create1(LabK, op5)); //LabK para label
		}
		else{
			//nao existe else no if, apenas colocamos a label pós if
			//cria a label Lx
			Operand op3 = {TempK, label1};
			ail_insert(ail_create1(LabK, op3)); //LabK para label
		}



      break;

      case VarK:
        //peguei a variavel, vou joga-la na arvore
        //criar um no na lista com essa variavel
        //Operand op1 = {SymtabK,10}
        //ail_insert(ail_create1(VarAsgk,op1));

        //ail_print();
      break;
      case FuncaoK:
        //printf("ENNTROU NO FUNCAOK %s\n", tree->child[0]->attr.name);

        //printf("FILHO 0: %s \n"tree->child[0]);
        printf("");

        int pos;
        pos = cgen_search_top(tree->child[0]->attr.name);
        Operand op2 = {SymtabK, pos};
        ail_insert(ail_create1(LabK, op2)); //LabK para label

		//chama o cgen novamente para os filhos da esquerda e direita
		cGen(tree->child[0]->child[1]);
      break;


         break;
		 default:
		 break;

    }
} /* genStmt */

/* Procedure genExp generates code at an expression node */
static void genExp( TreeNode * tree)
{ int loc;
  TreeNode * p1, * p2;
  switch (tree->kind.exp) {
	case ConstK :
		//printf("ENTROU NO CONSTK");
		printf("");
		OpGlobal.value = tree->attr.val;
      break; // ConstK

	/*
    case ConstK :

      if (TraceCode) emitComment("-> Const") ;
      // gen code to load integer constant using LDC
      emitRM("LDC",ac,tree->attr.val,0,"load const");
      if (TraceCode)  emitComment("<- Const") ;
      break; // ConstK

	  */
	case IntK:
		printf("ENTROU NO INTK");
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
		     ail_insert(ail_create1(FunctionParameterK, op2));
		 }
		 else{
			 if(strcmp(t->attr.type,"Integer") == 0){
				Operand op2 = {ImmK, t->attr.val};
				ail_insert(ail_create1(FunctionParameterK, op2));
			 }
			 else{
				Operand op2 = {SymtabK, cgen_search_top(t->attr.name)};
				ail_insert(ail_create1(FunctionParameterK, op2));
			 }
		 }
		 qt = qt+1;
		 t = t->sibling;
		}
		int funcao = cgen_search_top(tree->attr.name);
		tempT++;

		Operand op1 = {TempK, funcao, qt, tempT};
		ail_insert(ail_create1(FunctionCallK, op1));

	break;
    case IdK :
		printf("");


		OpGlobal.value = cgen_search_top(tree->attr.name);
      break; /* IdK */

	case OpK:
		//variavel tree->child[0]->attr.name valor tree->child[1]->attr.val

		//printf("ENTROU NO OPK >>>%s<<<\n",tree->child[0]->attr.name);

		printf("");
		int result;
		if(tree->attr.op == PLUS){
			cGen(tree->child[0]);
			//pode ser um id, Integer
			int var1 = OpGlobal.value;
			int tempVar1 = 0;
			int tempVar2 = 0;
			int tfVar1 = 0;
			int tfVar2 = 0;
			cGen(tree->child[1]);
			int var2 = OpGlobal.value;
			if(tree->child[0]->attr.type == NULL){
				//é um temporário, pega o ultimo temporario
				var1 = tempT;
				tempVar1 = 1;
			}
			else{
				//precisamos saber se é imediato ou variavel
				if (strcmp(tree->child[0]->attr.type,"id") == 0){
					//é um id
					tempVar1 = -1;
				}
			}
			if(tree->child[1]->attr.type == NULL){
				//é um temporário, pega o ultimo temporario
				var2 = tempT;
				tempVar2 = 1;
			}
			else{
				//precisamos saber se é imediato ou variavel
				if (strcmp(tree->child[1]->attr.type,"id") == 0){
					//é um id
					tempVar2 = -1;
				}
			}
			tempT++;
			Operand op1 = {TempK, var1, var2, tempT, tempVar1, tempVar2};
			ail_insert(ail_create1(AddK, op1)); //LabK para label
		}
		else if(tree->attr.op == MINUS){


      cGen(tree->child[0]);
      //pode ser um id, Integer
      int var1 = OpGlobal.value;
      int tempVar1 = 0;
      int tempVar2 = 0;
      int tfVar1 = 0;
      int tfVar2 = 0;
      cGen(tree->child[1]);
      int var2 = OpGlobal.value;
      if(tree->child[0]->attr.type == NULL){
        //é um temporário, pega o ultimo temporario
        var1 = tempT;
        tempVar1 = 1;
      }
      else{
        //precisamos saber se é imediato ou variavel
        if (strcmp(tree->child[0]->attr.type,"id") == 0){
          //é um id
          tempVar1 = -1;
        }
      }
      if(tree->child[1]->attr.type == NULL){
        //é um temporário, pega o ultimo temporario
        var2 = tempT;
        tempVar2 = 1;
      }
      else{
        //precisamos saber se é imediato ou variavel
        if (strcmp(tree->child[1]->attr.type,"id") == 0){
          //é um id
          tempVar2 = -1;
        }
      }
      tempT++;
      Operand op1 = {TempK, var1, var2, tempT, tempVar1, tempVar2};
      ail_insert(ail_create1(SubK, op1)); //LabK para label

		}
    else if(tree->attr.op == TIMES){
      cGen(tree->child[0]);
      //pode ser um id, Integer
      int var1 = OpGlobal.value;
      int tempVar1 = 0;
      int tempVar2 = 0;
      int tfVar1 = 0;
      int tfVar2 = 0;
      cGen(tree->child[1]);
      int var2 = OpGlobal.value;
      if(tree->child[0]->attr.type == NULL){
        //é um temporário, pega o ultimo temporario
        var1 = tempT;
        tempVar1 = 1;
      }
      else{
        //precisamos saber se é imediato ou variavel
        if (strcmp(tree->child[0]->attr.type,"id") == 0){
          //é um id
          tempVar1 = -1;
        }
      }
      if(tree->child[1]->attr.type == NULL){
        //é um temporário, pega o ultimo temporario
        var2 = tempT;
        tempVar2 = 1;
      }
      else{
        //precisamos saber se é imediato ou variavel
        if (strcmp(tree->child[1]->attr.type,"id") == 0){
          //é um id
          tempVar2 = -1;
        }
      }
      tempT++;
      Operand op1 = {TempK, var1, var2, tempT, tempVar1, tempVar2};
      ail_insert(ail_create1(MultK, op1)); //LabK para label

    }
		else if(tree->attr.op == OVER){
      cGen(tree->child[0]);
      //pode ser um id, Integer
      int var1 = OpGlobal.value;
      int tempVar1 = 0;
      int tempVar2 = 0;
      int tfVar1 = 0;
      int tfVar2 = 0;
      cGen(tree->child[1]);
      int var2 = OpGlobal.value;
      if(tree->child[0]->attr.type == NULL){
        //é um temporário, pega o ultimo temporario
        var1 = tempT;
        tempVar1 = 1;
      }
      else{
        //precisamos saber se é imediato ou variavel
        if (strcmp(tree->child[0]->attr.type,"id") == 0){
          //é um id
          tempVar1 = -1;
        }
      }
      if(tree->child[1]->attr.type == NULL){
        //é um temporário, pega o ultimo temporario
        var2 = tempT;
        tempVar2 = 1;
      }
      else{
        //precisamos saber se é imediato ou variavel
        if (strcmp(tree->child[1]->attr.type,"id") == 0){
          //é um id
          tempVar2 = -1;
        }
      }
      tempT++;
      Operand op1 = {TempK, var1, var2, tempT, tempVar1, tempVar2};
      ail_insert(ail_create1(DivK, op1)); //LabK para label

		}
		else if(tree->attr.op == EQ){

			//printf("EQ: %s\n",tree->child[1]->child[0]->attr.type);

			//tree->child[0]->attr.name recebendo outro opk
			int var1 = cgen_search_top(tree->child[0]->attr.name);//buscando a variavel que vai receber o assign
			//verificar se a variavel é um vetor
			int vec = 0;
			int posvec;
			if(strcmp(tree->child[0]->attr.typeVar, "vector") == 0){
				//printf("ÉPEEPE´´EÉ UM VETOR");

				//var1 é um vetor
				vec = 1;
				int posvec = tree->child[0]->child[0]->attr.val;

				  if(strcmp(tree->child[0]->child[0]->attr.type,"Integer")!=0){
					  //pos do vetor é variavel
					  //printf("POS DO VETOR: %d", tree->child[1]->child[0]->attr.val);
					  posvec = cgen_search_top(tree->child[0]->child[0]->attr.name);
				  }

			}
			if(tree->child[1]->attr.type == NULL){
				//assing é um opk
				cGen(tree->child[1]);
				if(vec == 0){
					Operand op1 = {TempK, var1, 0, tempT};
					ail_insert(ail_create1(VarAsgK, op1)); //LabK para label
				}
				else{
					//é um vetor, devemos buscar a posicao dele
					printf("POS DO VEC: %d",posvec);
					Operand op1 = {TempK, var1, posvec, tempT};
					ail_insert(ail_create1(VecAsgK, op1)); //LabK para label
				}

			}
			else{
				//é uma variavel ou uma chamada de funcao ou um inteiro
				if(strcmp(tree->child[1]->attr.type,"Integer")==0){
					//é um inteiro
					if(vec == 0){
						Operand op1 = {ImmK, var1, 0, tree->child[1]->attr.val};
						ail_insert(ail_create1(VarAsgK, op1));
					}
					else{
						Operand op1 = {ImmK, var1, posvec, tree->child[1]->attr.val};
						ail_insert(ail_create1(VecAsgK, op1));
					}
				}
				else{
					//aqui chama o idk
					cGen(tree->child[1]);
					//é uma variavel ou uma chamada de funcao
					if(strcmp(tree->child[1]->attr.type,"id")==0){

						//é uma variavel
						if(strcmp(tree->child[1]->attr.typeVar, "vector") == 0){
						  //é um vetor, necessario pegar a posicao dsejada
						  //printf("POS DO VETOR: %d", tree->child[1]->child[0]->attr.val);
						  int posv = tree->child[1]->child[0]->attr.val;
						   
						  if(strcmp(tree->child[1]->child[0]->attr.type,"Integer")!=0){
							  //pos do vetor é variavel
							  //printf("POS DO VETOR: %d", tree->child[1]->child[0]->attr.val);
							  posv = cgen_search_top(tree->child[1]->child[0]->attr.name);
							  Operand op1 = {VecK, var1, posv, cgen_search_top(tree->child[1]->attr.name),0,0,SymtabK};
							  ail_insert(ail_create1(VarAsgK, op1));
						  }
						  else{
							  Operand op1 = {VecK, var1, posv, cgen_search_top(tree->child[1]->attr.name),0,0,ImmK};
							  ail_insert(ail_create1(VarAsgK, op1));
						  }
							
						}

						else{
						  //é uma variável
							if(vec == 0){
								Operand op1 = {SymtabK, var1, 0, cgen_search_top(tree->child[1]->attr.name)};
								ail_insert(ail_create1(VarAsgK, op1));
							}
							else{
								Operand op1 = {SymtabK, var1, posvec, cgen_search_top(tree->child[1]->attr.name)};
								ail_insert(ail_create1(VecAsgK, op1));
							}
						}
					}
					else{
							//é uma chamada de funcao
						cGen(tree->child[1]);
						if(vec == 0){
							Operand op1 = {TempK, var1, 0, tempT};
							ail_insert(ail_create1(VarAsgK, op1));
						}
						else{
							Operand op1 = {TempK, var1, posvec, tempT};
							ail_insert(ail_create1(VecAsgK, op1));
						}
					}

				}
			}

		}
		else{
			//comparações
			//preciso buscar a pos da variavel
			int pos;
			pos = cgen_search_top(tree->child[0]->attr.name);
			//pode ser um inteiro, uma variavel ou uma chamada de funcao

			if(tree->child[1]->attr.type == NULL){//comparando com outro temporário (outro OpK)
				cGen(tree->child[1]);
				Operand op1 = {TempK, pos, tempT, tempT+1};
				tempT++;
				if(tree->attr.op == EQEQ) //==
					ail_insert(ail_create1(CmpEqK, op1));
				else if(tree->attr.op == NEQ) //!=
					ail_insert(ail_create1(CmpNEqK, op1));
				else if(tree->attr.op == LT) // <
					ail_insert(ail_create1(CmpLK, op1));
				else if(tree->attr.op == LE) // <=
					ail_insert(ail_create1(CmpLEqK, op1));
				else if(tree->attr.op == MT) // >
					ail_insert(ail_create1(CmpGK, op1));
				else if(tree->attr.op == ME) // >=
					ail_insert(ail_create1(CmpGEqK, op1));
			}
			else{
				if(strcmp(tree->child[1]->attr.type,"Integer")==0){ //comparando com inteiro
					tempT++;
					Operand op1 = {ImmK, pos, tree->child[1]->attr.val, tempT};

				}
				else{
					//comparando com função ou variavel
					if (strcmp(tree->child[1]->attr.type,"id") == 0){
						//e uma variavel
						int pos2 = cgen_search_top(tree->child[1]->attr.name);
						//resta descobrir se é vetor ou nao
						if(strcmp(tree->child[1]->attr.typeVar, "vector") == 0){
							Operand op1 = {VecK, pos, pos2, tempT+1,tree->child[1]->child[0]->attr.val};
							tempT++;
						}
						else{
							Operand op1 = {SymtabK, pos, pos2, tempT+1};
							tempT++;
						}
						
					}
					else{
						//é uma funcao
					}
				}
				
				if(tree->attr.op == EQEQ) //==
					ail_insert(ail_create1(CmpEqK, op1));
					else if(tree->attr.op == NEQ) //!=
						ail_insert(ail_create1(CmpNEqK, op1));
					else if(tree->attr.op == LT) // <
						ail_insert(ail_create1(CmpLK, op1));
					else if(tree->attr.op == LE) // <=
						ail_insert(ail_create1(CmpLEqK, op1));
					else if(tree->attr.op == MT) // >
						ail_insert(ail_create1(CmpGK, op1));
					else if(tree->attr.op == ME) // >=
						ail_insert(ail_create1(CmpGEqK, op1));
			}
			//printf("POS: %d\n", pos);
			//Operand op1 = {TempK, pos, tree->child[1]->attr.val, 1};
			//ail_insert(ail_create1(CmpEqK, op1)); //LabK para label

		}

		//printf("VARIAVEL: %s RECEBE: %d\n",tree->child[0]->attr.name, tree->child[1]->attr.val);
	break;
    default:
      break;
  }
} /* genExp */

/* Procedure cGen recursively generates code by
 * tree traversal
 */
static void cGen( TreeNode * tree)
{
  //printf("ENTROU NO CGEN\n");
  if (tree != NULL)
  {

    switch (tree->nodekind) {
      /*aki1*/

      case StmtK:
        //printf("ENTROU NO GENSTMT\n");
        genStmt(tree);
        break;
      case ExpK:
      //printf("ENTROU NO EXPK\n");

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
