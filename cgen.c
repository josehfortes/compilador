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
int escopoAtual;


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
int posAsl = 0;

AssemblyList asl_create(Instruction ins, AssemblyOperand aso){
    AssemblyList as = (AssemblyList) malloc(sizeof(struct AssemblyList));
    as->ins = ins;
    as->op1 = aso;
    as->pos = posAsl;
    posAsl++;
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

int getVarFromMemory(int pos, int tam){
	//temos a posicao has pos
	//buscamos a posicao na memoria dessa variavel
	int posmem = pos;
	//posmem é a posicao de memoria, devmeos usar um registrador para ler a memoria
	int reg1 = busca_reg_livre();
	posmem += tam;
    //usamos o addi
	AssemblyOperand op1 = {reg1,decimal_binario(posmem)};//posicao de memoria da variavel
	asl_insert(asl_create(ADDI, op1));
	//efetuamos um LW somando com o REG_0 e
	AssemblyOperand op2 = {reg1, reg1};
	asl_insert(asl_create(LW, op2));
	//printf("LW REG_%d, MEMORY[REG_0 + %d]\n", reg, posmem);
	//limpa_reg(reg1);
	return reg1;
	//pronto, o reg possui o valor que estava na memoria agr
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

int localPos = 0;

int busca_tamanho_assembly(){
	AssemblyList t = asl;
	int i=0;
	while(t != NULL){
		t = t->next;
		i++;
	}
	return i;
}

int busca_funcao_assembly(int lab){
	AssemblyList t = asl;
	int i=0;
	while(t != NULL){
		if(t->op1.value == lab)
			return i;
		t = t->next;
		i++;
	}
	return -1;
}

AssemblyOperand op;
void gera_assembly(){
  AsmInstList t = ail;
  int posmem1;
  int posmem2;
  int reg1 = 0;
  int reg2 = 0;
  int reg3 = 0;
  int reg4 = 0;

  while(t != NULL){
		switch(t->aik){
		case AsgK:
			reg1 = busca_reg_livre();//vai receber a posicao da memoria a ser armazenado
			reg2 = busca_reg_livre();

			posmem1 = t->op1.value;
			//pode ser uma variavel ou um vetor
			if(t->op1.kind == VarAsgK){
				//é uma variavel
				AssemblyOperand op = {reg1,decimal_binario(posmem1)};
				asl_insert(asl_create(ADDI, op));
			}
			else{
				//é um vetor
				if(t->op1.type == ImmK){
					posmem1 += t->op1.tam;
					AssemblyOperand op = {reg1,decimal_binario(posmem1)};
					asl_insert(asl_create(ADDI, op));
				}
				else{
					//o vetor possui variavel como valor da posicao ex.:v[i]
					posmem2 = t->op1.tam;
					//posmem2 representa a posicao da variavel, devemos dar um LW em um registrador REG3 esse valor
					reg3 = busca_reg_livre();
					//fazemos um addi no reg3 com o posmem2
					AssemblyOperand op1 = {reg3,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op1));
					//fazemos o lw
					AssemblyOperand op2 = {reg3, reg3};
					asl_insert(asl_create(LW, op2));
					//agora, devemos somar o reg3 com o reg1
					//criamos um addi com a pos do reg1 no reg1
					AssemblyOperand op3 = {reg1,decimal_binario(posmem1)};
					asl_insert(asl_create(ADDI, op3));
					//criamos um add entre esses 2 registradores e armazenamos no proprio reg1
					AssemblyOperand op4 = {reg1,reg3, reg1};
					asl_insert(asl_create(ADD, op4));
					limpa_reg(reg3);
				}
			}

			//vamos ver o que a variavel ira aramzenar
			if(t->op2.kind == ImmK){
				//é um imediato
				//precisamso de armazenar em um registrador reg2 o valor desse imediato
				AssemblyOperand op = {reg2,decimal_binario(t->op2.value)};
				asl_insert(asl_create(ADDI, op));
			}
			else if (t->op2.kind == TempK){
				AssemblyOperand op = {0,31,reg2};
				asl_insert(asl_create(ADD, op));
			}
			else if (t->op2.kind == VecK){
				//é um vetor
				//a posicao do vetor é t->op2.value, precisamos de somar esse valor com a posicao do tamanho
				//precisamos verificar se o tamanho do vetor é um imediato ou variavel
				posmem2 = t->op2.value;
				if(t->op2.type == ImmK){
					posmem2 += t->op2.tam;
					AssemblyOperand op = {reg2,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op));
					//fazemos o lw
					AssemblyOperand op2 = {reg2, reg2};
					asl_insert(asl_create(LW, op2));
				}
				else{
					//o operando é uma variavel, precismaos de armazenar ela em um vetor, somar com o posmem2 e só depois dar o lw
					//fazemos um addi com o posmem2
					AssemblyOperand op = {reg2,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op));
					//buscamos o valor da variavel
					reg3 = busca_reg_livre();
					//fazemos um addi com a variavel no vetor reg3
					AssemblyOperand op3 = {reg3,decimal_binario(t->op2.tam)};
					asl_insert(asl_create(ADDI, op3));
					//fazemos o lw
					AssemblyOperand op2 = {reg3, reg3};
					asl_insert(asl_create(LW, op2));
					//somamos os dois e armazenamos no reg2
					AssemblyOperand op4 = {reg2,reg3, reg2};
					asl_insert(asl_create(ADD, op4));
					//liberamos o reg3
					limpa_reg(reg3);
				}
			}
			else{
				//é uma variavel
				reg2 = busca_reg_livre();
				posmem1 = t->op2.value;
				AssemblyOperand op = {reg2,decimal_binario(posmem1)};
				asl_insert(asl_create(ADDI, op));
				//fazemos o lw
				AssemblyOperand op2 = {reg2, reg2};
				asl_insert(asl_create(LW, op2));
			}
			//criamos o store
			AssemblyOperand op = {reg2,reg1};
			asl_insert(asl_create(STORE, op));
			//liberamos os 2 registradores
			limpa_reg(reg1);
			limpa_reg(reg2);
		break;
		case AddK:
			//precisamos saber se é imediato, variavel, funcao, temporario
			/*
			na soma propriamente dita, só será somado dois registradores, logo, precisamos de pegar os dois numeros e armazenar em um reg
			*/

			//inicio para a variavel 1
			if(t->op1.kind == ImmK){
				//é um imediato, devemos armazenar o seu valor ( em binario ) em um registrador livre
				//buscamos um registrador livre
				reg1 = busca_reg_livre();
				//somamos um imediato a esse registrador
				AssemblyOperand op1 = {reg1,decimal_binario(t->op1.value)};
				asl_insert(asl_create(ADDI, op1));
			}
			else if(t->op1.kind == TempK){//é um temporario
				reg1 = 31;
			}
			else if(t->op1.kind == VecK){
				//é vetor =(
				reg1 = getVarFromMemory(t->op1.value, t->op1.tam);
			}
			else{
				//é variavel
				reg1 = getVarFromMemory(t->op1.value, 0);
			}
			//fim para a variavel 1
			//inicio pra varaivel 2
			if(t->op2.kind == ImmK){
				//é um imediato, devemos armazenar o seu valor ( em binario ) em um registrador livre
				//buscamos um registrador livre
				reg2 = busca_reg_livre();
				//somamos um imediato a esse registrador
				AssemblyOperand op2 = {reg2,decimal_binario(t->op2.value)};
				asl_insert(asl_create(ADDI, op2));
			}
			else if(t->op2.kind == TempK){//é um temporario
				reg2 = 31;
			}
			else if(t->op2.kind == VecK){
				//é vetor =(
				reg2 = getVarFromMemory(t->op2.value, t->op2.tam);
			}
			else{
				//é variavel
				reg2 = getVarFromMemory(t->op2.value, 0);
			}
			//fim pra variavle 2
			//devemos fazer um add e armazenar em um terceiro registrador
			reg3 = 31;
			AssemblyOperand op3 = {reg1,reg2, reg3};
			asl_insert(asl_create(ADD, op3));
			limpa_reg(reg1);
			limpa_reg(reg2);

		break;
		case SubK:
		//precisamos saber se é imediato, variavel, funcao, temporario
			/*
			na soma propriamente dita, só será somado dois registradores, logo, precisamos de pegar os dois numeros e armazenar em um reg
			*/

			//inicio para a variavel 1
			if(t->op1.kind == ImmK){
				//é um imediato, devemos armazenar o seu valor ( em binario ) em um registrador livre
				//buscamos um registrador livre
				reg1 = busca_reg_livre();
				//somamos um imediato a esse registrador
				AssemblyOperand op1 = {reg1,decimal_binario(t->op1.value)};
				asl_insert(asl_create(ADDI, op1));
			}
			else if(t->op1.kind == TempK){//é um temporario
				reg1 = 31;
			}
			else if(t->op1.kind == VecK){
				//é vetor =(
				reg1 = getVarFromMemory(t->op1.value, t->op1.tam);
			}
			else{
				//é variavel
				reg1 = getVarFromMemory(t->op1.value, 0);
			}
			//fim para a variavel 1
			//inicio pra varaivel 2
			if(t->op2.kind == ImmK){
				//é um imediato, devemos armazenar o seu valor ( em binario ) em um registrador livre
				//buscamos um registrador livre
				reg2 = busca_reg_livre();
				//somamos um imediato a esse registrador
				AssemblyOperand op2 = {reg2,decimal_binario(t->op2.value)};
				asl_insert(asl_create(ADDI, op2));
			}
			else if(t->op2.kind == TempK){//é um temporario
				reg2 = 31;
			}
			else if(t->op2.kind == VecK){
				//é vetor =(
				reg2 = getVarFromMemory(t->op2.value, t->op2.tam);
			}
			else{
				//é variavel
				reg2 = getVarFromMemory(t->op2.value, 0);
			}
			//fim pra variavle 2
			//devemos fazer um add e armazenar em um terceiro registrador
			reg3 = 31;
			AssemblyOperand op4 = {reg1,reg2, reg3};
			asl_insert(asl_create(SUB, op4));
			limpa_reg(reg1);
			limpa_reg(reg2);
		break;
		case MultK:
		//precisamos saber se é imediato, variavel, funcao, temporario
			/*
			na soma propriamente dita, só será somado dois registradores, logo, precisamos de pegar os dois numeros e armazenar em um reg
			*/

			//inicio para a variavel 1
			if(t->op1.kind == ImmK){
				//é um imediato, devemos armazenar o seu valor ( em binario ) em um registrador livre
				//buscamos um registrador livre
				reg1 = busca_reg_livre();
				//somamos um imediato a esse registrador
				AssemblyOperand op1 = {reg1,decimal_binario(t->op1.value)};
				asl_insert(asl_create(ADDI, op1));
			}
			else if(t->op1.kind == TempK){//é um temporario
				reg1 = 31;
			}
			else if(t->op1.kind == VecK){
				//é vetor =(
				reg1 = getVarFromMemory(t->op1.value, t->op1.tam);
			}
			else{
				//é variavel
				reg1 = getVarFromMemory(t->op1.value, 0);
			}
			//fim para a variavel 1
			//inicio pra varaivel 2
			if(t->op2.kind == ImmK){
				//é um imediato, devemos armazenar o seu valor ( em binario ) em um registrador livre
				//buscamos um registrador livre
				reg2 = busca_reg_livre();
				//somamos um imediato a esse registrador
				AssemblyOperand op2 = {reg2,decimal_binario(t->op2.value)};
				asl_insert(asl_create(ADDI, op2));
			}
			else if(t->op2.kind == TempK){//é um temporario
				reg2 = 31;
			}
			else if(t->op2.kind == VecK){
				//é vetor =(
				reg2 = getVarFromMemory(t->op2.value, t->op2.tam);
			}
			else{
				//é variavel
				reg2 = getVarFromMemory(t->op2.value, 0);
			}
			//fim pra variavle 2
			//devemos fazer um add e armazenar em um terceiro registrador
			reg3 = 31;
			op.value = reg1;
			op.value2 = reg2;
			op.value3 = reg3;

			asl_insert(asl_create(MULT, op));
			limpa_reg(reg1);
			limpa_reg(reg2);
		break;
		case DivK:
		//precisamos saber se é imediato, variavel, funcao, temporario
			/*
			na soma propriamente dita, só será somado dois registradores, logo, precisamos de pegar os dois numeros e armazenar em um reg
			*/

			//inicio para a variavel 1
			if(t->op1.kind == ImmK){
				//é um imediato, devemos armazenar o seu valor ( em binario ) em um registrador livre
				//buscamos um registrador livre
				reg1 = busca_reg_livre();
				//somamos um imediato a esse registrador
				AssemblyOperand op1 = {reg1,decimal_binario(t->op1.value)};
				asl_insert(asl_create(ADDI, op1));
			}
			else if(t->op1.kind == TempK){//é um temporario
				reg1 = 31;
			}
			else if(t->op1.kind == VecK){
				//é vetor =(
				reg1 = getVarFromMemory(t->op1.value, t->op1.tam);
			}
			else{
				//é variavel
				reg1 = getVarFromMemory(t->op1.value, 0);
			}
			//fim para a variavel 1
			//inicio pra varaivel 2
			if(t->op2.kind == ImmK){
				//é um imediato, devemos armazenar o seu valor ( em binario ) em um registrador livre
				//buscamos um registrador livre
				reg2 = busca_reg_livre();
				//somamos um imediato a esse registrador
				AssemblyOperand op2 = {reg2,decimal_binario(t->op2.value)};
				asl_insert(asl_create(ADDI, op2));
			}
			else if(t->op2.kind == TempK){//é um temporario
				reg2 = 31;
			}
			else if(t->op2.kind == VecK){
				//é vetor =(
				reg2 = getVarFromMemory(t->op2.value, t->op2.tam);
			}
			else{
				//é variavel
				reg2 = getVarFromMemory(t->op2.value, 0);
			}
			//fim pra variavle 2
			//devemos fazer um add e armazenar em um terceiro registrador
			reg3 = 31;
			op.value = reg1;
			op.value2 = reg2;
			op.value3 = reg3;

			asl_insert(asl_create(DIV, op));
			limpa_reg(reg1);
			limpa_reg(reg2);

		break;
		case CmpEqK:
			reg1 = busca_reg_livre();//vai receber a posicao da memoria a ser armazenado
			reg2 = busca_reg_livre();
			posmem1 = t->op1.value;
			//pode ser uma variavel ou um vetor

			if(t->op1.kind == VecK){
				//é um vetor
				if(t->op1.type == ImmK){
					posmem1 += t->op1.tam;
					AssemblyOperand op = {reg1,decimal_binario(posmem1)};
					asl_insert(asl_create(ADDI, op));
				}
				else{
					//o vetor possui variavel como valor da posicao ex.:v[i]
					posmem2 = t->op1.tam;
					//posmem2 representa a posicao da variavel, devemos dar um LW em um registrador REG3 esse valor
					reg3 = busca_reg_livre();
					//fazemos um addi no reg3 com o posmem2
					AssemblyOperand op1 = {reg3,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op1));
					//fazemos o lw
					AssemblyOperand op2 = {reg3, reg3};
					asl_insert(asl_create(LW, op2));
					//agora, devemos somar o reg3 com o reg1
					//criamos um addi com a pos do reg1 no reg1
					AssemblyOperand op3 = {reg1,decimal_binario(posmem1)};
					asl_insert(asl_create(ADDI, op3));
					//criamos um add entre esses 2 registradores e armazenamos no proprio reg1
					AssemblyOperand op4 = {reg1,reg3, reg1};
					asl_insert(asl_create(ADD, op4));
					limpa_reg(reg3);
				}
			}
			else{
				//é uma variavel
				AssemblyOperand op = {reg1,decimal_binario(posmem1)};
				asl_insert(asl_create(ADDI, op));
			}
			//precisamos de buscar essa variavel dando um LW e armazenando no proprio registrador
			op.value = reg1;
			op.value2 = reg1;
			asl_insert(asl_create(LW, op));

			//vamos ver o que a variavel ira aramzenar
			if(t->op2.kind == ImmK){
				//é um imediato
				//precisamso de armazenar em um registrador reg2 o valor desse imediato
				AssemblyOperand op = {reg2,decimal_binario(t->op2.value)};
				asl_insert(asl_create(ADDI, op));
			}
			else if (t->op2.kind == TempK){
				AssemblyOperand op = {0,31,reg2};
				asl_insert(asl_create(ADD, op));
			}
			else if (t->op2.kind == VecK){
				//é um vetor
				//a posicao do vetor é t->op2.value, precisamos de somar esse valor com a posicao do tamanho
				//precisamos verificar se o tamanho do vetor é um imediato ou variavel
				posmem2 = t->op2.value;
				if(t->op2.type == ImmK){
					posmem2 += t->op2.tam;
					AssemblyOperand op = {reg2,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op));
					//fazemos o lw
					AssemblyOperand op2 = {reg2, reg2};
					asl_insert(asl_create(LW, op2));
				}
				else{
					//o operando é uma variavel, precismaos de armazenar ela em um vetor, somar com o posmem2 e só depois dar o lw
					//fazemos um addi com o posmem2
					AssemblyOperand op = {reg2,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op));
					//buscamos o valor da variavel
					reg3 = busca_reg_livre();
					//fazemos um addi com a variavel no vetor reg3
					AssemblyOperand op3 = {reg3,decimal_binario(t->op2.tam)};
					asl_insert(asl_create(ADDI, op3));
					//fazemos o lw
					AssemblyOperand op2 = {reg3, reg3};
					asl_insert(asl_create(LW, op2));
					//somamos os dois e armazenamos no reg2
					AssemblyOperand op4 = {reg2,reg3, reg2};
					asl_insert(asl_create(ADD, op4));
					//liberamos o reg3
					limpa_reg(reg3);
				}
			}
			else{
				//é uma variavel
				reg2 = busca_reg_livre();
				posmem1 = t->op2.value;
				AssemblyOperand op = {reg2,decimal_binario(posmem1)};
				asl_insert(asl_create(ADDI, op));
				//fazemos o lw
				AssemblyOperand op2 = {reg2, reg2};
				asl_insert(asl_create(LW, op2));
			}


			//criamos a comparacao e se for verdade pula 2 casas
			op.value = reg1;
			op.value2 = reg2;
			op.value3 = posAsl+2;
			asl_insert(asl_create(BEQ, op));
			//liberamos os 2 registradores
			limpa_reg(reg1);
			limpa_reg(reg2);
		break;
		case CmpNEqK:
			reg1 = busca_reg_livre();//vai receber a posicao da memoria a ser armazenado
			reg2 = busca_reg_livre();
			posmem1 = t->op1.value;
			//pode ser uma variavel ou um vetor

			if(t->op1.kind == VecK){
				//é um vetor
				if(t->op1.type == ImmK){
					posmem1 += t->op1.tam;
					AssemblyOperand op = {reg1,decimal_binario(posmem1)};
					asl_insert(asl_create(ADDI, op));
				}
				else{
					//o vetor possui variavel como valor da posicao ex.:v[i]
					posmem2 = t->op1.tam;
					//posmem2 representa a posicao da variavel, devemos dar um LW em um registrador REG3 esse valor
					reg3 = busca_reg_livre();
					//fazemos um addi no reg3 com o posmem2
					AssemblyOperand op1 = {reg3,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op1));
					//fazemos o lw
					AssemblyOperand op2 = {reg3, reg3};
					asl_insert(asl_create(LW, op2));
					//agora, devemos somar o reg3 com o reg1
					//criamos um addi com a pos do reg1 no reg1
					AssemblyOperand op3 = {reg1,decimal_binario(posmem1)};
					asl_insert(asl_create(ADDI, op3));
					//criamos um add entre esses 2 registradores e armazenamos no proprio reg1
					AssemblyOperand op4 = {reg1,reg3, reg1};
					asl_insert(asl_create(ADD, op4));
					limpa_reg(reg3);
				}
			}
			else{
				//é uma variavel
				AssemblyOperand op = {reg1,decimal_binario(posmem1)};
				asl_insert(asl_create(ADDI, op));
			}
			//precisamos de buscar essa variavel dando um LW e armazenando no proprio registrador
			op.value = reg1;
			op.value2 = reg1;
			asl_insert(asl_create(LW, op));

			//vamos ver o que a variavel ira aramzenar
			if(t->op2.kind == ImmK){
				//é um imediato
				//precisamso de armazenar em um registrador reg2 o valor desse imediato
				AssemblyOperand op = {reg2,decimal_binario(t->op2.value)};
				asl_insert(asl_create(ADDI, op));
			}
			else if (t->op2.kind == TempK){
				AssemblyOperand op = {0,31,reg2};
				asl_insert(asl_create(ADD, op));
			}
			else if (t->op2.kind == VecK){
				//é um vetor
				//a posicao do vetor é t->op2.value, precisamos de somar esse valor com a posicao do tamanho
				//precisamos verificar se o tamanho do vetor é um imediato ou variavel
				posmem2 = t->op2.value;
				if(t->op2.type == ImmK){
					posmem2 += t->op2.tam;
					AssemblyOperand op = {reg2,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op));
					//fazemos o lw
					AssemblyOperand op2 = {reg2, reg2};
					asl_insert(asl_create(LW, op2));
				}
				else{
					//o operando é uma variavel, precismaos de armazenar ela em um vetor, somar com o posmem2 e só depois dar o lw
					//fazemos um addi com o posmem2
					AssemblyOperand op = {reg2,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op));
					//buscamos o valor da variavel
					reg3 = busca_reg_livre();
					//fazemos um addi com a variavel no vetor reg3
					AssemblyOperand op3 = {reg3,decimal_binario(t->op2.tam)};
					asl_insert(asl_create(ADDI, op3));
					//fazemos o lw
					AssemblyOperand op2 = {reg3, reg3};
					asl_insert(asl_create(LW, op2));
					//somamos os dois e armazenamos no reg2
					AssemblyOperand op4 = {reg2,reg3, reg2};
					asl_insert(asl_create(ADD, op4));
					//liberamos o reg3
					limpa_reg(reg3);
				}
			}
			else{
				//é uma variavel
				reg2 = busca_reg_livre();
				posmem1 = t->op2.value;
				AssemblyOperand op = {reg2,decimal_binario(posmem1)};
				asl_insert(asl_create(ADDI, op));
				//fazemos o lw
				AssemblyOperand op2 = {reg2, reg2};
				asl_insert(asl_create(LW, op2));
			}


			//criamos a comparacao e se for verdade pula 2 casas
			op.value = reg1;
			op.value2 = reg2;
			op.value3 = posAsl+2;
			asl_insert(asl_create(BNE, op));
			//liberamos os 2 registradores
			limpa_reg(reg1);
			limpa_reg(reg2);
		break;
		case CmpGK:
			reg1 = busca_reg_livre();//vai receber a posicao da memoria a ser armazenado
			reg2 = busca_reg_livre();
			posmem1 = t->op1.value;
			//pode ser uma variavel ou um vetor

			if(t->op1.kind == VecK){
				//é um vetor
				if(t->op1.type == ImmK){
					posmem1 += t->op1.tam;
					AssemblyOperand op = {reg1,decimal_binario(posmem1)};
					asl_insert(asl_create(ADDI, op));
				}
				else{
					//o vetor possui variavel como valor da posicao ex.:v[i]
					posmem2 = t->op1.tam;
					//posmem2 representa a posicao da variavel, devemos dar um LW em um registrador REG3 esse valor
					reg3 = busca_reg_livre();
					//fazemos um addi no reg3 com o posmem2
					AssemblyOperand op1 = {reg3,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op1));
					//fazemos o lw
					AssemblyOperand op2 = {reg3, reg3};
					asl_insert(asl_create(LW, op2));
					//agora, devemos somar o reg3 com o reg1
					//criamos um addi com a pos do reg1 no reg1
					AssemblyOperand op3 = {reg1,decimal_binario(posmem1)};
					asl_insert(asl_create(ADDI, op3));
					//criamos um add entre esses 2 registradores e armazenamos no proprio reg1
					AssemblyOperand op4 = {reg1,reg3, reg1};
					asl_insert(asl_create(ADD, op4));
					limpa_reg(reg3);
				}
			}
			else{
				//é uma variavel
				AssemblyOperand op = {reg1,decimal_binario(posmem1)};
				asl_insert(asl_create(ADDI, op));
			}
			//precisamos de buscar essa variavel dando um LW e armazenando no proprio registrador
			op.value = reg1;
			op.value2 = reg1;
			asl_insert(asl_create(LW, op));

			//vamos ver o que a variavel ira aramzenar
			if(t->op2.kind == ImmK){
				//é um imediato
				//precisamso de armazenar em um registrador reg2 o valor desse imediato
				AssemblyOperand op = {reg2,decimal_binario(t->op2.value)};
				asl_insert(asl_create(ADDI, op));
			}
			else if (t->op2.kind == TempK){
				AssemblyOperand op = {0,31,reg2};
				asl_insert(asl_create(ADD, op));
			}
			else if (t->op2.kind == VecK){
				//é um vetor
				//a posicao do vetor é t->op2.value, precisamos de somar esse valor com a posicao do tamanho
				//precisamos verificar se o tamanho do vetor é um imediato ou variavel
				posmem2 = t->op2.value;
				if(t->op2.type == ImmK){
					posmem2 += t->op2.tam;
					AssemblyOperand op = {reg2,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op));
					//fazemos o lw
					AssemblyOperand op2 = {reg2, reg2};
					asl_insert(asl_create(LW, op2));
				}
				else{
					//o operando é uma variavel, precismaos de armazenar ela em um vetor, somar com o posmem2 e só depois dar o lw
					//fazemos um addi com o posmem2
					AssemblyOperand op = {reg2,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op));
					//buscamos o valor da variavel
					reg3 = busca_reg_livre();
					//fazemos um addi com a variavel no vetor reg3
					AssemblyOperand op3 = {reg3,decimal_binario(t->op2.tam)};
					asl_insert(asl_create(ADDI, op3));
					//fazemos o lw
					AssemblyOperand op2 = {reg3, reg3};
					asl_insert(asl_create(LW, op2));
					//somamos os dois e armazenamos no reg2
					AssemblyOperand op4 = {reg2,reg3, reg2};
					asl_insert(asl_create(ADD, op4));
					//liberamos o reg3
					limpa_reg(reg3);
				}
			}
			else{
				//é uma variavel
				reg2 = busca_reg_livre();
				posmem1 = t->op2.value;
				AssemblyOperand op = {reg2,decimal_binario(posmem1)};
				asl_insert(asl_create(ADDI, op));
				//fazemos o lw
				AssemblyOperand op2 = {reg2, reg2};
				asl_insert(asl_create(LW, op2));
			}


			//criamos a comparacao e se for verdade pula 2 casas
			op.value = reg1;
			op.value2 = reg2;
			op.value3 = posAsl+2;
			asl_insert(asl_create(BGT, op));
			//liberamos os 2 registradores
			limpa_reg(reg1);
			limpa_reg(reg2);
		break;
		case CmpLK:
		reg1 = busca_reg_livre();//vai receber a posicao da memoria a ser armazenado
			reg2 = busca_reg_livre();
			posmem1 = t->op1.value;
			//pode ser uma variavel ou um vetor

			if(t->op1.kind == VecK){
				//é um vetor
				if(t->op1.type == ImmK){
					posmem1 += t->op1.tam;
					AssemblyOperand op = {reg1,decimal_binario(posmem1)};
					asl_insert(asl_create(ADDI, op));
				}
				else{
					//o vetor possui variavel como valor da posicao ex.:v[i]
					posmem2 = t->op1.tam;
					//posmem2 representa a posicao da variavel, devemos dar um LW em um registrador REG3 esse valor
					reg3 = busca_reg_livre();
					//fazemos um addi no reg3 com o posmem2
					AssemblyOperand op1 = {reg3,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op1));
					//fazemos o lw
					AssemblyOperand op2 = {reg3, reg3};
					asl_insert(asl_create(LW, op2));
					//agora, devemos somar o reg3 com o reg1
					//criamos um addi com a pos do reg1 no reg1
					AssemblyOperand op3 = {reg1,decimal_binario(posmem1)};
					asl_insert(asl_create(ADDI, op3));
					//criamos um add entre esses 2 registradores e armazenamos no proprio reg1
					AssemblyOperand op4 = {reg1,reg3, reg1};
					asl_insert(asl_create(ADD, op4));
					limpa_reg(reg3);
				}
			}
			else{
				//é uma variavel
				AssemblyOperand op = {reg1,decimal_binario(posmem1)};
				asl_insert(asl_create(ADDI, op));
			}
			//precisamos de buscar essa variavel dando um LW e armazenando no proprio registrador
			op.value = reg1;
			op.value2 = reg1;
			asl_insert(asl_create(LW, op));

			//vamos ver o que a variavel ira aramzenar
			if(t->op2.kind == ImmK){
				//é um imediato
				//precisamso de armazenar em um registrador reg2 o valor desse imediato
				AssemblyOperand op = {reg2,decimal_binario(t->op2.value)};
				asl_insert(asl_create(ADDI, op));
			}
			else if (t->op2.kind == TempK){
				AssemblyOperand op = {0,31,reg2};
				asl_insert(asl_create(ADD, op));
			}
			else if (t->op2.kind == VecK){
				//é um vetor
				//a posicao do vetor é t->op2.value, precisamos de somar esse valor com a posicao do tamanho
				//precisamos verificar se o tamanho do vetor é um imediato ou variavel
				posmem2 = t->op2.value;
				if(t->op2.type == ImmK){
					posmem2 += t->op2.tam;
					AssemblyOperand op = {reg2,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op));
					//fazemos o lw
					AssemblyOperand op2 = {reg2, reg2};
					asl_insert(asl_create(LW, op2));
				}
				else{
					//o operando é uma variavel, precismaos de armazenar ela em um vetor, somar com o posmem2 e só depois dar o lw
					//fazemos um addi com o posmem2
					AssemblyOperand op = {reg2,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op));
					//buscamos o valor da variavel
					reg3 = busca_reg_livre();
					//fazemos um addi com a variavel no vetor reg3
					AssemblyOperand op3 = {reg3,decimal_binario(t->op2.tam)};
					asl_insert(asl_create(ADDI, op3));
					//fazemos o lw
					AssemblyOperand op2 = {reg3, reg3};
					asl_insert(asl_create(LW, op2));
					//somamos os dois e armazenamos no reg2
					AssemblyOperand op4 = {reg2,reg3, reg2};
					asl_insert(asl_create(ADD, op4));
					//liberamos o reg3
					limpa_reg(reg3);
				}
			}
			else{
				//é uma variavel
				reg2 = busca_reg_livre();
				posmem1 = t->op2.value;
				AssemblyOperand op = {reg2,decimal_binario(posmem1)};
				asl_insert(asl_create(ADDI, op));
				//fazemos o lw
				AssemblyOperand op2 = {reg2, reg2};
				asl_insert(asl_create(LW, op2));
			}


			//criamos a comparacao e se for verdade pula 2 casas
			op.value = reg1;
			op.value2 = reg2;
			op.value3 = posAsl+2;
			asl_insert(asl_create(BLT, op));
			//liberamos os 2 registradores
			limpa_reg(reg1);
			limpa_reg(reg2);
		break;
		case CmpGEqK:
		reg1 = busca_reg_livre();//vai receber a posicao da memoria a ser armazenado
			reg2 = busca_reg_livre();
			posmem1 = t->op1.value;
			//pode ser uma variavel ou um vetor

			if(t->op1.kind == VecK){
				//é um vetor
				if(t->op1.type == ImmK){
					posmem1 += t->op1.tam;
					AssemblyOperand op = {reg1,decimal_binario(posmem1)};
					asl_insert(asl_create(ADDI, op));
				}
				else{
					//o vetor possui variavel como valor da posicao ex.:v[i]
					posmem2 = t->op1.tam;
					//posmem2 representa a posicao da variavel, devemos dar um LW em um registrador REG3 esse valor
					reg3 = busca_reg_livre();
					//fazemos um addi no reg3 com o posmem2
					AssemblyOperand op1 = {reg3,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op1));
					//fazemos o lw
					AssemblyOperand op2 = {reg3, reg3};
					asl_insert(asl_create(LW, op2));
					//agora, devemos somar o reg3 com o reg1
					//criamos um addi com a pos do reg1 no reg1
					AssemblyOperand op3 = {reg1,decimal_binario(posmem1)};
					asl_insert(asl_create(ADDI, op3));
					//criamos um add entre esses 2 registradores e armazenamos no proprio reg1
					AssemblyOperand op4 = {reg1,reg3, reg1};
					asl_insert(asl_create(ADD, op4));
					limpa_reg(reg3);
				}
			}
			else{
				//é uma variavel
				AssemblyOperand op = {reg1,decimal_binario(posmem1)};
				asl_insert(asl_create(ADDI, op));
			}
			//precisamos de buscar essa variavel dando um LW e armazenando no proprio registrador
			op.value = reg1;
			op.value2 = reg1;
			asl_insert(asl_create(LW, op));

			//vamos ver o que a variavel ira aramzenar
			if(t->op2.kind == ImmK){
				//é um imediato
				//precisamso de armazenar em um registrador reg2 o valor desse imediato
				AssemblyOperand op = {reg2,decimal_binario(t->op2.value)};
				asl_insert(asl_create(ADDI, op));
			}
			else if (t->op2.kind == TempK){
				AssemblyOperand op = {0,31,reg2};
				asl_insert(asl_create(ADD, op));
			}
			else if (t->op2.kind == VecK){
				//é um vetor
				//a posicao do vetor é t->op2.value, precisamos de somar esse valor com a posicao do tamanho
				//precisamos verificar se o tamanho do vetor é um imediato ou variavel
				posmem2 = t->op2.value;
				if(t->op2.type == ImmK){
					posmem2 += t->op2.tam;
					AssemblyOperand op = {reg2,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op));
					//fazemos o lw
					AssemblyOperand op2 = {reg2, reg2};
					asl_insert(asl_create(LW, op2));
				}
				else{
					//o operando é uma variavel, precismaos de armazenar ela em um vetor, somar com o posmem2 e só depois dar o lw
					//fazemos um addi com o posmem2
					AssemblyOperand op = {reg2,decimal_binario(posmem2)};
					asl_insert(asl_create(ADDI, op));
					//buscamos o valor da variavel
					reg3 = busca_reg_livre();
					//fazemos um addi com a variavel no vetor reg3
					AssemblyOperand op3 = {reg3,decimal_binario(t->op2.tam)};
					asl_insert(asl_create(ADDI, op3));
					//fazemos o lw
					AssemblyOperand op2 = {reg3, reg3};
					asl_insert(asl_create(LW, op2));
					//somamos os dois e armazenamos no reg2
					AssemblyOperand op4 = {reg2,reg3, reg2};
					asl_insert(asl_create(ADD, op4));
					//liberamos o reg3
					limpa_reg(reg3);
				}
			}
			else{
				//é uma variavel
				reg2 = busca_reg_livre();
				posmem1 = t->op2.value;
				AssemblyOperand op = {reg2,decimal_binario(posmem1)};
				asl_insert(asl_create(ADDI, op));
				//fazemos o lw
				AssemblyOperand op2 = {reg2, reg2};
				asl_insert(asl_create(LW, op2));
			}


			//criamos a comparacao e se for verdade pula 2 casas
			op.value = reg1;
			op.value2 = reg2;
			op.value3 = posAsl+3;
			asl_insert(asl_create(BGT, op));
			op.value = reg1;
			op.value2 = reg2;
			op.value3 = posAsl+2;
			asl_insert(asl_create(BEQ, op));
			//liberamos os 2 registradores
			limpa_reg(reg1);
			limpa_reg(reg2);
		break;

		case CmpLEqK:
    reg1 = busca_reg_livre();//vai receber a posicao da memoria a ser armazenado
      reg2 = busca_reg_livre();
      posmem1 = t->op1.value;
      //pode ser uma variavel ou um vetor

      if(t->op1.kind == VecK){
        //é um vetor
        if(t->op1.type == ImmK){
          posmem1 += t->op1.tam;
          AssemblyOperand op = {reg1,decimal_binario(posmem1)};
          asl_insert(asl_create(ADDI, op));
        }
        else{
          //o vetor possui variavel como valor da posicao ex.:v[i]
          posmem2 = t->op1.tam;
          //posmem2 representa a posicao da variavel, devemos dar um LW em um registrador REG3 esse valor
          reg3 = busca_reg_livre();
          //fazemos um addi no reg3 com o posmem2
          AssemblyOperand op1 = {reg3,decimal_binario(posmem2)};
          asl_insert(asl_create(ADDI, op1));
          //fazemos o lw
          AssemblyOperand op2 = {reg3, reg3};
          asl_insert(asl_create(LW, op2));
          //agora, devemos somar o reg3 com o reg1
          //criamos um addi com a pos do reg1 no reg1
          AssemblyOperand op3 = {reg1,decimal_binario(posmem1)};
          asl_insert(asl_create(ADDI, op3));
          //criamos um add entre esses 2 registradores e armazenamos no proprio reg1
          AssemblyOperand op4 = {reg1,reg3, reg1};
          asl_insert(asl_create(ADD, op4));
          limpa_reg(reg3);
        }
      }
      else{
        //é uma variavel
        AssemblyOperand op = {reg1,decimal_binario(posmem1)};
        asl_insert(asl_create(ADDI, op));
      }
      //precisamos de buscar essa variavel dando um LW e armazenando no proprio registrador
      op.value = reg1;
      op.value2 = reg1;
      asl_insert(asl_create(LW, op));

      //vamos ver o que a variavel ira aramzenar
      if(t->op2.kind == ImmK){
        //é um imediato
        //precisamso de armazenar em um registrador reg2 o valor desse imediato
        AssemblyOperand op = {reg2,decimal_binario(t->op2.value)};
        asl_insert(asl_create(ADDI, op));
      }
      else if (t->op2.kind == TempK){
        AssemblyOperand op = {0,31,reg2};
        asl_insert(asl_create(ADD, op));
      }
      else if (t->op2.kind == VecK){
        //é um vetor
        //a posicao do vetor é t->op2.value, precisamos de somar esse valor com a posicao do tamanho
        //precisamos verificar se o tamanho do vetor é um imediato ou variavel
        posmem2 = t->op2.value;
        if(t->op2.type == ImmK){
          posmem2 += t->op2.tam;
          AssemblyOperand op = {reg2,decimal_binario(posmem2)};
          asl_insert(asl_create(ADDI, op));
          //fazemos o lw
          AssemblyOperand op2 = {reg2, reg2};
          asl_insert(asl_create(LW, op2));
        }
        else{
          //o operando é uma variavel, precismaos de armazenar ela em um vetor, somar com o posmem2 e só depois dar o lw
          //fazemos um addi com o posmem2
          AssemblyOperand op = {reg2,decimal_binario(posmem2)};
          asl_insert(asl_create(ADDI, op));
          //buscamos o valor da variavel
          reg3 = busca_reg_livre();
          //fazemos um addi com a variavel no vetor reg3
          AssemblyOperand op3 = {reg3,decimal_binario(t->op2.tam)};
          asl_insert(asl_create(ADDI, op3));
          //fazemos o lw
          AssemblyOperand op2 = {reg3, reg3};
          asl_insert(asl_create(LW, op2));
          //somamos os dois e armazenamos no reg2
          AssemblyOperand op4 = {reg2,reg3, reg2};
          asl_insert(asl_create(ADD, op4));
          //liberamos o reg3
          limpa_reg(reg3);
        }
      }
      else{
        //é uma variavel
        reg2 = busca_reg_livre();
        posmem1 = t->op2.value;
        AssemblyOperand op = {reg2,decimal_binario(posmem1)};
        asl_insert(asl_create(ADDI, op));
        //fazemos o lw
        AssemblyOperand op2 = {reg2, reg2};
        asl_insert(asl_create(LW, op2));
      }


      //criamos a comparacao e se for verdade pula 2 casas
      op.value = reg1;
      op.value2 = reg2;
      op.value3 = posAsl+3;
      asl_insert(asl_create(BLT, op));
      op.value = reg1;
      op.value2 = reg2;
      op.value3 = posAsl+2;
      asl_insert(asl_create(BEQ, op));
      //liberamos os 2 registradores
      limpa_reg(reg1);
      limpa_reg(reg2);
		break;
		case LabK:
			op.value = t->op1.value;
			asl_insert(asl_create(LAB, op));
		break;
		case GotoK:
			op.value = t->op1.value;
			asl_insert(asl_create(GOTO, op));
		break;
		case If_FK:
			//criamos o goto que vai virar jump pra label
			op.value = t->op2.value;
			asl_insert(asl_create(GOTO, op));
		break;
		case FunctionParameterK:
			//iniciamos alocando um registador e armazenando nele o valor do endereço da variavel onde salvaremos
			//o endereço da variavel econtra-se no t->op2.value
			reg1 = busca_reg_livre();
			reg2 = busca_reg_livre();
			//somamos um imediato a esse registrador
			AssemblyOperand op2 = {reg2,decimal_binario(t->op2.value)};
			asl_insert(asl_create(ADDI, op2));
			//agora vamos verificar o que é a variavel que será armazenado
			if(t->op1.kind == ImmK){
				//é um imediato
				AssemblyOperand op1 = {reg1,decimal_binario(t->op1.value)};
				asl_insert(asl_create(ADDI, op1));
			}
			else if(t->op1.kind == TempK){
				AssemblyOperand op1 = {reg1,31,reg1};
				asl_insert(asl_create(ADD, op1));
			}
			else if(t->op1.kind == VecK){
				//é um vetor

			}
			else{
				//é uma variavel
				//somamos o seu valor no reg1
				AssemblyOperand op4 = {reg1,decimal_binario(t->op1.value)};
				asl_insert(asl_create(ADDI, op4));
				//carregamos com lw e armazenamos no proprio reg1
				AssemblyOperand op5 = {reg1, reg1};
				asl_insert(asl_create(LW, op5));
			}

			//fazemos um store agora para armazenar na posição correta o parametro
			AssemblyOperand op6 = {reg1, reg2};
			asl_insert(asl_create(STORE, op6));
			//liberamos os 2 registradores
			limpa_reg(reg1);
			limpa_reg(reg2);

			/*
			if(t->op1.kind == VecK){
				sprintf(str, "%d[%d]", t->op1.value, t->op1.tam);
				if(t->op1.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op1.value, t->op1.tam);
				}
			}
			else if(t->op1.kind == TempK)
				sprintf(str, "t%d", t->op1.value);
			else if(t->op1.kind == ImmK)
				sprintf(str, "(%d)", t->op1.value);
			printf("(par,%s, %d)\n",str, t->op2.value);
			*/
		break;
		case FunctionCallK:
			//a funcao possui a hash t->op1.value - procuraremos qual posicao de memoria ela se encontra
			reg1 = busca_reg_livre();
			reg2 = busca_reg_livre();
			op.value = reg1;
			op.value2 = busca_funcao_memoria(t->op1.value);
			asl_insert(asl_create(ADDI, op));
			//reg1 vai receber o valor da posicao de memoria que será gravada a posicao do jump no return
			//precisamos buscar a posição que ele voltará para o jmpr e armazenar nessa posicao de memoria
			op.value = reg2;
			op.value2 = busca_tamanho_assembly() + 3;
			asl_insert(asl_create(ADDI, op));
			//fazemos o store
			op.value = reg2;
			op.value2 = reg1;
			asl_insert(asl_create(STORE, op));
			limpa_reg(reg1);
			limpa_reg(reg2);
			//agora faremos o jump pra funcao, precisamos de buscar a posicao que encontra-se a label da funcao
			op.value = busca_funcao_assembly(t->op1.value) + 1;
			asl_insert(asl_create(JMP, op));
		break;
		case FunctionReturnK:
      //precisamos de pegar o valor do return - pode ser um temporario, inteiro, funcao, vetor, variavel, etc
      //armazenaremos o retorno no registrador 31

      if(t->op1.kind == ImmK){
        //é imediato
			op.value = 31;
  			op.value2 = t->op1.value;
  			asl_insert(asl_create(ADDI, op));
      }
      else if(t->op1.kind == SymtabK){
        //é variavel
        reg1 = busca_reg_livre();
  			op.value = reg1;
  			op.value2 = t->op1.value;
  			asl_insert(asl_create(ADDI, op));
        op.value = reg1;
  			op.value2 = reg1;
  			asl_insert(asl_create(LW, op));
        //add reg 1 com reg 31
        op.value = reg1;
        op.value = 31;
        op.value3 = 31;
        asl_insert(asl_create(ADD, op));
      }
      else if(t->op1.kind == TempK){
        //é temporario
        //está no reg 31 já
      }
      else{
        //é vetor

      }

			//precisamos de recuperar a posicao na memoria pra dar o jmpr - a funcao possui hash t->op2.value
			reg1 = busca_reg_livre();
			op.value = reg1;
			op.value2 = busca_funcao_memoria(t->op2.value);
			asl_insert(asl_create(ADDI, op));
			op.value = reg1;
			op.value2 = reg1;
			asl_insert(asl_create(LW, op));
			asl_insert(asl_create(JMPR, op));
			limpa_reg(reg1);
		break;
		case InputK:
			op.value = 31;
			asl_insert(asl_create(IN, op));
		break;
		case OutputK:
			//addi com a var1
			op.value = 31;
			op.value2 = t->op1.value;
			asl_insert(asl_create(ADDI, op));
			op.value2 = 31;
			asl_insert(asl_create(LW, op));
			op.value3 = 31;
			asl_insert(asl_create(OUT3, op));
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
	  printf("POS: [%d] - ",t->pos);
    switch(t->ins){
      case ADD:
        printf("ADD REG_%d, REG%d + REG%d\n",t->op1.value3,t->op1.value,t->op1.value2);
      break;
	    case ADDI:
        printf("ADDI REG_%d, %d\n",t->op1.value,t->op1.value2);
      break;
      case SUB:
        printf("SUB REG_%d, REG%d - REG%d\n",t->op1.value3,t->op1.value,t->op1.value2);
      break;
      case MULT:
        printf("MULT REG_%d, REG%d * REG%d\n",t->op1.value3,t->op1.value,t->op1.value2);
      break;
      case DIV:
        printf("DIV REG_%d, REG%d / REG%d\n",t->op1.value3,t->op1.value,t->op1.value2);
      break;
      case STORE:
        printf("STORE REG_%d, MEMORY[REG_%d]\n", t->op1.value,t->op1.value2);
      break;
      case LW:
        printf("LW REG_%d, MEMORY[REG_%d]\n", t->op1.value,t->op1.value2);
	  break;
	  case LAB:
        printf("LAB %d\n", t->op1.value);
	  break;
	  case GOTO:
        printf("GOTO LAB%d\n", t->op1.value);
      break;
	  case JMP:
		printf("JMP %d\n", t->op1.value);
	  break;
	  case JMPR:
		printf("JMPR REG_%d\n", t->op1.value);
	  break;
	  case BEQ:
		printf("BEQ REG_%d, REG_%d GO TO %d\n", t->op1.value, t->op1.value2,t->op1.value3);
	  break;
	  case BNE:
		printf("BNE REG_%d, REG_%d GO TO %d\n", t->op1.value, t->op1.value2,t->op1.value3);
	  break;
	  case BGT:
		printf("BGT REG_%d, REG_%d GO TO %d\n", t->op1.value, t->op1.value2,t->op1.value3);
	  break;
	  case BLT:
		printf("BLT REG_%d, REG_%d GO TO %d\n", t->op1.value, t->op1.value2,t->op1.value3);
	  break;
	  case IN:
		printf("IN REG_%d\n", t->op1.value);
	  break;
	  case OUT:
		printf("OUT REG_%d\n", t->op1.value);
	  break;
	  case OUT3:
		printf("OUT3 REG_%d REG_%d REG_%d\n", t->op1.value,t->op1.value2,t->op1.value3);
	  break;
    }
    t = t->next;
  }

  printf("------------------------------------------------------\n");
}

int buscaLab(int l){
  AssemblyList t = asl;
  while(t != NULL){
    if(t->ins == LAB)
      if(t->op1.value == l)
        return t->pos;
    t = t->next;
  }

}

void gera_txt(){
  char string[100];
  char cont[100];
  FILE *arq;
  arq = fopen("binario.txt", "wt");  // Cria um arquivo texto para gravação
  AssemblyList t = asl;
  while(t != NULL){

    if(t->pos == 1){
      //busca a main
      sprintf(cont, "{6'b011100, 26'd%d}; //ir para main",buscaLab(24));
    }
    else
    switch(t->ins){
      case ADD:
        //printf("ADD REG_%d, REG%d + REG%d\n",t->op1.value3,t->op1.value,t->op1.value2);
        sprintf(cont, "{6'd000000, 5'd%d, 5'd%d, 5'd%d, 11'd0}; //add",t->op1.value, t->op1.value2, t->op1.value3);
      break;
	    case ADDI:
        //printf("ADDI REG_%d, %d\n",t->op1.value,t->op1.value2);
        sprintf(cont, "{6'b000001, 5'd%d, 21'd%d}; //addi",t->op1.value, t->op1.value2);
      break;
      case SUB:
        //printf("SUB REG_%d, REG%d - REG%d\n",t->op1.value3,t->op1.value,t->op1.value2);
		sprintf(cont, "{6'd000010, 5'd%d, 5'd%d, 5'd%d, 11'd0}; //add",t->op1.value, t->op1.value2, t->op1.value3);
      break;
      case MULT:
        sprintf(cont, "{6'b000100, 5'd%d, 5'd%d, 5'd%d, 11'd0}; //mult",t->op1.value, t->op1.value2, t->op1.value3);
        //printf("MULT REG_%d, REG%d * REG%d\n",t->op1.value3,t->op1.value,t->op1.value2);
      break;
      case DIV:
        sprintf(cont, "{6'b000101, 5'd%d, 5'd%d, 5'd%d, 11'd0}; //div",t->op1.value, t->op1.value2, t->op1.value3);
        //printf("DIV REG_%d, REG%d / REG%d\n",t->op1.value3,t->op1.value,t->op1.value2);
      break;
      case STORE:
        sprintf(cont, "{6'b100001, 5'd%d, 5'd%d, 16'd0}; //store",t->op1.value, t->op1.value2);
      break;
      case LW:
        sprintf(cont, "{6'b100000, 5'd%d, 5'd%d, 16'd0}; //lw",t->op1.value, t->op1.value2);
        //printf("LW REG_%d, MEMORY[REG_%d]\n", t->op1.value,t->op1.value2);
	  break;
	  case LAB:
        //transforma pra halt
        sprintf(cont, "{6'b101000, 26'd0}; //nop");
        //printf("LAB %d\n", t->op1.value);
	  break;
	  case GOTO:
        sprintf(cont, "{6'b011100, 26'd%d}; //goto",buscaLab(t->op1.value));
        //printf("GOTO LAB%d\n", t->op1.value);
    break;
	  case JMP:
      sprintf(cont, "{6'b011100, 26'd%d}; //jump",t->op1.value);
		//printf("JMP %d\n", t->op1.value);
	  break;
	  case JMPR:
    sprintf(cont, "{6'b011101, 5'd%d, 21'd0}; //jmpr",t->op1.value);
		//printf("JMPR REG_%d\n", t->op1.value);
	  break;
	  case BEQ:
    sprintf(cont, "{6'b011000, 5'd%d, 5'd%d, 16'd%d}; //beq",t->op1.value, t->op1.value2, t->op1.value3);
		//printf("BEQ REG_%d, REG_%d GO TO %d\n", t->op1.value, t->op1.value2,t->op1.value3);
	  break;
	  case BNE:
    sprintf(cont, "{6'b011001, 5'd%d, 5'd%d, 16'd%d}; //bne",t->op1.value, t->op1.value2, t->op1.value3);
		//printf("BNE REG_%d, REG_%d GO TO %d\n", t->op1.value, t->op1.value2,t->op1.value3);
	  break;
	  case BGT:
    sprintf(cont, "{6'b011010, 5'd%d, 5'd%d, 16'd%d}; //bgt",t->op1.value, t->op1.value2, t->op1.value3);
		//printf("BGT REG_%d, REG_%d GO TO %d\n", t->op1.value, t->op1.value2,t->op1.value3);
	  break;
	  case BLT:
    sprintf(cont, "{6'b011011, 5'd%d, 5'd%d, 16'd%d}; //blt",t->op1.value, t->op1.value2, t->op1.value3);
		//printf("BLT REG_%d, REG_%d GO TO %d\n", t->op1.value, t->op1.value2,t->op1.value3);
	  break;
	  case IN:
    sprintf(cont, "{6'b111000, 5'd%d, 21'd0}; //in",t->op1.value);
		//printf("IN REG_%d\n", t->op1.value);
	  break;
	  case OUT:
		//printf("OUT REG_%d\n", t->op1.value);
	  break;
	  case OUT3:
    sprintf(cont, "{6'b111010, 5'd%d, 5'd%d, 5'd%d, 11'd0}; //out3",t->op1.value, t->op1.value2, t->op1.value3);
		//printf("OUT3 REG_%d REG_%d REG_%d\n", t->op1.value,t->op1.value2,t->op1.value3);
	  break;

    default:
      strcpy(cont, "32'b000000_00000_00000_00000_xxxxxxxxxxxxxxx");
    break;
    }

    sprintf(string, "mem_instrucoes[%d] = %s\n",t->pos, cont);
    strcpy(cont,"");
    fputs(string, arq);
    t = t->next;
  }
  fclose(arq);
}

void ail_print(){
	char str[100];
	char str2[100];
  AsmInstList t = ail;
  printf("\n--------- CODIGO INTERMEDIARIO ---------\n");
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
			if(t->op1.kind == VecK){
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
		case CmpEqK:
			sprintf(str, "%d", t->op1.value);
			sprintf(str2, "%d", t->op2.value);
			//verificando a variavel 1
			if(t->op1.kind == VecK){
				sprintf(str, "%d[%d]", t->op1.value, t->op1.tam);
				if(t->op1.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op1.value, t->op1.tam);
				}
			}
			//verificando a variavel 2
			if(t->op2.kind == TempK)
				sprintf(str2, "t%d", t->op2.value);
			else if (t->op2.kind == ImmK)
				sprintf(str2, "(%d)", t->op2.value);
			else if(t->op2.kind == VecK){
				sprintf(str, "%d[%d]", t->op2.value, t->op2.tam);
				if(t->op2.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op2.value, t->op2.tam);
				}
			}

			printf("(Eq,%s,%s,t%d)\n",str,str2,t->op3.value);
		break;
		case CmpNEqK:
			sprintf(str, "%d", t->op1.value);
			sprintf(str2, "%d", t->op2.value);
			//verificando a variavel 1
			if(t->op1.kind == VecK){
				sprintf(str, "%d[%d]", t->op1.value, t->op1.tam);
				if(t->op1.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op1.value, t->op1.tam);
				}
			}
			//verificando a variavel 2
			if(t->op2.kind == TempK)
				sprintf(str2, "t%d", t->op2.value);
			else if (t->op2.kind == ImmK)
				sprintf(str2, "(%d)", t->op2.value);
			else if(t->op2.kind == VecK){
				sprintf(str, "%d[%d]", t->op2.value, t->op2.tam);
				if(t->op2.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op2.value, t->op2.tam);
				}
			}

			printf("(Neq,%s,%s,t%d)\n",str,str2,t->op3.value);
		break;
		case CmpGK:
			sprintf(str, "%d", t->op1.value);
			sprintf(str2, "%d", t->op2.value);
			//verificando a variavel 1
			if(t->op1.kind == VecK){
				sprintf(str, "%d[%d]", t->op1.value, t->op1.tam);
				if(t->op1.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op1.value, t->op1.tam);
				}
			}
			//verificando a variavel 2
			if(t->op2.kind == TempK)
				sprintf(str2, "t%d", t->op2.value);
			else if (t->op2.kind == ImmK)
				sprintf(str2, "(%d)", t->op2.value);
			else if(t->op2.kind == VecK){
				sprintf(str, "%d[%d]", t->op2.value, t->op2.tam);
				if(t->op2.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op2.value, t->op2.tam);
				}
			}

			printf("(Gt,%s,%s,t%d)\n",str,str2,t->op3.value);
		break;
		case CmpGEqK:
			sprintf(str, "%d", t->op1.value);
			sprintf(str2, "%d", t->op2.value);
			//verificando a variavel 1
			if(t->op1.kind == VecK){
				sprintf(str, "%d[%d]", t->op1.value, t->op1.tam);
				if(t->op1.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op1.value, t->op1.tam);
				}
			}
			//verificando a variavel 2
			if(t->op2.kind == TempK)
				sprintf(str2, "t%d", t->op2.value);
			else if (t->op2.kind == ImmK)
				sprintf(str2, "(%d)", t->op2.value);
			else if(t->op2.kind == VecK){
				sprintf(str, "%d[%d]", t->op2.value, t->op2.tam);
				if(t->op2.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op2.value, t->op2.tam);
				}
			}

			printf("(Get,%s,%s,t%d)\n",str,str2,t->op3.value);
		break;
		case CmpLK:
			sprintf(str, "%d", t->op1.value);
			sprintf(str2, "%d", t->op2.value);
			//verificando a variavel 1
			if(t->op1.kind == VecK){
				sprintf(str, "%d[%d]", t->op1.value, t->op1.tam);
				if(t->op1.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op1.value, t->op1.tam);
				}
			}
			//verificando a variavel 2
			if(t->op2.kind == TempK)
				sprintf(str2, "t%d", t->op2.value);
			else if (t->op2.kind == ImmK)
				sprintf(str2, "(%d)", t->op2.value);
			else if(t->op2.kind == VecK){
				sprintf(str, "%d[%d]", t->op2.value, t->op2.tam);
				if(t->op2.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op2.value, t->op2.tam);
				}
			}

			printf("(Lt,%s,%s,t%d)\n",str,str2,t->op3.value);
		break;
		case CmpLEqK:
			sprintf(str, "%d", t->op1.value);
			sprintf(str2, "%d", t->op2.value);
			//verificando a variavel 1
			if(t->op1.kind == VecK){
				sprintf(str, "%d[%d]", t->op1.value, t->op1.tam);
				if(t->op1.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op1.value, t->op1.tam);
				}
			}
			//verificando a variavel 2
			if(t->op2.kind == TempK)
				sprintf(str2, "t%d", t->op2.value);
			else if (t->op2.kind == ImmK)
				sprintf(str2, "(%d)", t->op2.value);
			else if(t->op2.kind == VecK){
				sprintf(str, "%d[%d]", t->op2.value, t->op2.tam);
				if(t->op2.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op2.value, t->op2.tam);
				}
			}

			printf("(Let,%s,%s,t%d)\n",str,str2,t->op3.value);
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
			printf("(cal,%d,%d,t%d)\n",t->op1.value,t->op2.value,t->op3.value);
		break;
		case InputK:
			printf("(IN,t%d,_,_)\n",t->op3.value);
		break;
		case OutputK:
			printf("(OUT,%d,_,_)\n",t->op1.value);
		break;
		case FunctionVoidCallK:
			printf("(Vcal,%d,%d,_)\n",t->op1.value, t->op2.value);
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
			printf("(if_f,t%d,L%d,_)\n",t->op1.value,t->op2.value);
		break;
		case FunctionParameterK:
			sprintf(str, "%d", t->op1.value);
			if(t->op1.kind == VecK){
				sprintf(str, "%d[%d]", t->op1.value, t->op1.tam);
				if(t->op1.type == ImmK){
					sprintf(str, "%d[(%d)]", t->op1.value, t->op1.tam);
				}
			}
			else if(t->op1.kind == TempK)
				sprintf(str, "t%d", t->op1.value);
			else if(t->op1.kind == ImmK)
				sprintf(str, "(%d)", t->op1.value);
			printf("(par,%s, %d)\n",str, t->op2.value);
		break;
    }

    t = t->next;
  }

  printf("------------------------------------------------------\n");
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
		if(tree->child[0]->attr.type == NULL){
			//temporario
			cGen(tree->child[0]);
 			Operand op2 = {TempK, tempT};
			Operand op3 = {TempK, cgen_search_top(nomeEscopoAtual)};
 			ail_insert(ail_create(FunctionReturnK, op2, op3, opn)); //FunctionReturnK para retorno de inteiro
		}
 		else if(strcmp(tree->child[0]->attr.type, "Integer") == 0){
 			Operand op2 = {ImmK, tree->child[0]->attr.val};
			Operand op3 = {TempK, cgen_search_top(nomeEscopoAtual)};
 			ail_insert(ail_create(FunctionReturnK, op2, op3, opn)); //FunctionReturnK para retorno de inteiro
 		}
 		else if(strcmp(tree->child[0]->attr.type, "id") == 0){
 			Operand op2 = {SymtabK, buscaEscopo(tree->child[0]->attr.name)};
			Operand op3 = {TempK, cgen_search_top(nomeEscopoAtual)};
 			ail_insert(ail_create(FunctionReturnK, op2, op3, opn)); //FunctionReturnK para retorno de variavel

 		}
 		else if(strcmp(tree->child[0]->attr.type, "funcao") == 0){
 			//é necessario acessar o ativk
 			cGen(tree->child[0]);
 			Operand op2 = {TempK, tempT};
			Operand op3 = {TempK, cgen_search_top(nomeEscopoAtual)};
 			ail_insert(ail_create(FunctionReturnK, op2, op3, opn)); //FunctionReturnK para retorno de inteiro
 		}
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
		nomeEscopoAtual = tree->child[0]->attr.name;
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
		printf("entrou no ativk\n");
		//precisamos buscar quantos parametros a funcao temp
		int qt = 0;
		TreeNode * t = tree->child[0];
		int nescopo = busca_parametro(tree->attr.name);
		int nvar;
		int out;
		while(t != NULL){
		nvar = busca_var_par(nescopo, qt);
		 if (t->attr.type == NULL){
			 cGen(t);
			 //parametro é o temporario
			 Operand op2 = {TempK, tempT};
			 Operand op3 = {AssignK, nvar};
		     ail_insert(ail_create(FunctionParameterK, op2, op3, opn));
		 }
		 else{
			 if(strcmp(t->attr.type,"Integer") == 0){
				Operand op3 = {AssignK, nvar};
				Operand op2 = {ImmK, t->attr.val};
				ail_insert(ail_create(FunctionParameterK, op2, op3, opn));
			 }
			 else{
				//pode ser uma variavel ou uma funcao
				if(strcmp(t->attr.type,"funcao") == 0){//é uma funcao
					cGen(t);
					Operand op2 = {TempK, tempT};
					Operand op3 = {AssignK, nvar};
					ail_insert(ail_create(FunctionParameterK, op2, op3, opn));
				}
				else{//é uma variavel
					//cGen(t);
					Operand op3 = {AssignK, nvar};
					Operand op2 = {SymtabK, buscaEscopo(t->attr.name)};
					//veriificar se é vetor ou n

					if(strcmp(t->attr.typeVar, "vector") == 0){
						//é um vetor
						op2.kind = VecK;
						//o value é o mesmo, precisamos de armazenar as informacoes do content agora

						int posvec2 = t->child[0]->attr.val;
						op2.tam = posvec2;
						op2.type = ImmK;


						if(strcmp(t->child[0]->attr.type,"Integer")!=0){
							//o valor de dentro do [] do vetor e uma variavel
							int posvec2 = buscaEscopo(t->child[0]->attr.name);
							op2.tam = posvec2;
							op2.type = SymtabK;
						}
					}
					ail_insert(ail_create(FunctionParameterK, op2, op3, opn));
				}

			 }
		 }
		 qt = qt+1;
		 t = t->sibling;

		}
		int funcao = cgen_search_top(tree->attr.name);
		tempT++;
		op3.value = tempT;
		Operand op1 = {TempK, funcao};
		op2.value = qt;

		if(strcmp(tree->attr.name, "output") == 0){
			op1.value = nvar;
			ail_insert(ail_create(OutputK, op1, op2, op3));
		}
		else
			ail_insert(ail_create(FunctionCallK, op1, op2, op3));

	break;
    case IdK :
		if(strcmp(tree->attr.type,"chamadaFuncao") == 0){
			//chamada de funcao void
			op1.value = cgen_search_top(tree->attr.name);
			//precisamos buscar quantos parametros a funcao temp
			op2.value = 0;
			tempT++;
			op3.value = tempT;

			if (strcmp(tree->attr.name, "input") == 0)
				ail_insert(ail_create(InputK, op1, op2, op3));
			else
				ail_insert(ail_create(FunctionCallK, op1, op2, op3));
		}
		else{
			OpGlobal.value = buscaEscopo(tree->attr.name);
		}
    break; /* IdK */
	case OpK:
		printf("entrou no OpK\n");
		//todas as operações

		//verifica se é um assign
		if(tree->attr.op == EQ){//é um assign, iremos no final criar o asgk

			int var1 = buscaEscopo(tree->child[0]->attr.name);//buscando a variavel que vai receber o assign
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
					posvec = buscaEscopo(tree->child[0]->child[0]->attr.name);
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
						op2.value = buscaEscopo(tree->child[1]->attr.name);
						if(strcmp(tree->child[1]->attr.typeVar, "vector") == 0){
							//é um vetor
							op2.kind = VecK;
							//o value é o mesmo, precisamos de armazenar as informacoes do content agora

							int posvec2 = tree->child[1]->child[0]->attr.val;
							op2.tam = posvec2;
							op2.type = ImmK;


							if(strcmp(tree->child[1]->child[0]->attr.type,"Integer")!=0){
								//o valor de dentro do [] do vetor e uma variavel
								int posvec2 = buscaEscopo(tree->child[1]->child[0]->attr.name);
								op2.tam = posvec2;
								op2.type = SymtabK;
							}

						}
					}
					else{
						//é uma chamada de funcao, devemos chama-la e armazenar dps o seu retorno
						//o retorno estará em um temporario
						//cGen(tree->child[1]);
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
						int posvec = buscaEscopo(tree->child[0]->child[0]->attr.name);
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
						int posvec = buscaEscopo(tree->child[1]->child[0]->attr.name);
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
			if(tree->attr.op == EQEQ) //==
				Op1 = CmpEqK;
			else if(tree->attr.op == NEQ) //!=
				Op1= CmpNEqK;
			else if(tree->attr.op == LT) // <
				Op1 = CmpLK;
			else if(tree->attr.op == LE) // <=
				Op1 = CmpLEqK;
			else if(tree->attr.op == MT) // >
				Op1 = CmpGK;
			else if(tree->attr.op == ME) // >=
				Op1 = CmpGEqK;
			//ja sabemos o que comparar, agora precisamos de verificar o primeiro elemento
			//ele pode ser uma variavel comum ou vetor
			op1.kind = SymtabK;
			op1.value = buscaEscopo(tree->child[0]->attr.name);
			if(strcmp(tree->child[0]->attr.typeVar, "vector") == 0){
				//é um vetor, devemos fazer todo o tratamento...
				op1.kind = VecK;
				int posvec = tree->child[0]->child[0]->attr.val;
				op1.tam = posvec;
				op1.type = ImmK;
				if(strcmp(tree->child[0]->child[0]->attr.type,"Integer")!=0){
					//o valor de dentro do [] do vetor e uma variavel
					int posvec = buscaEscopo(tree->child[0]->child[0]->attr.name);
					op1.tam = posvec;
					op1.type = SymtabK;
				}
			}
			//ja temos a primeira variavel,a gora devemos tratar a segunda
			//a segunda variavel pode ser: imediato, variavel(comum ou vetor), opk ou chamada de funcao
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
						op2.value = buscaEscopo(tree->child[1]->attr.name);
						if(strcmp(tree->child[1]->attr.typeVar, "vector") == 0){
							//é um vetor
							op2.kind = VecK;
							//o value é o mesmo, precisamos de armazenar as informacoes do content agora

							int posvec2 = tree->child[1]->child[0]->attr.val;
							op2.tam = posvec2;
							op2.type = ImmK;


							if(strcmp(tree->child[1]->child[0]->attr.type,"Integer")!=0){
								//o valor de dentro do [] do vetor e uma variavel
								int posvec2 = buscaEscopo(tree->child[1]->child[0]->attr.name);
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
			//precisamos setar um temporario pra instrucao e armazena-lo, este vai no op3
			tempT++;
			op3.value = tempT;
			//criamos na lista
			ail_insert(ail_create(Op1,op1,op2,op3));

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
	for(int i=1; i<32;i++){
		registradores[i] = 0;
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
   gera_txt();
}
