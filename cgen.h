/****************************************************/
/* File: cgen.h                                     */
/* The code generator interface to the TINY compiler*/
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _CGEN_H_
#define _CGEN_H_

 /* Kinds of Operands */
 typedef enum {TempK, SymtabK, ImmK, VecK, FuncK} OperandKind;
 /* Assembly Instruction Set: 21 instructions */
 typedef enum {
                 AddK, SubK, MultK, DivK,
                 CmpEqK, CmpNEqK, CmpGK, CmpGEqK, CmpLK, CmpLEqK,
                 AsgK, VarAsgK, VecAsgK,
                 InputK, OutputK,
                 FunctionParameterK, FunctionCallK, FunctionReturnK, LabK, FunctionVoidCallK,
                 GotoK,If_FK
 } AsmInstKind;

 typedef struct Operand {
	OperandKind kind;
	int value;
	//usado para vetores
	int tam;
	OperandKind type;
 } Operand;

 typedef struct AsmInstList {
     struct AsmInstList * next;
     AsmInstKind aik;
     Operand op1;
     Operand op2;
	 Operand op3;
 } * AsmInstList;
 AsmInstList ail;

 typedef enum {
   ADD, ADDI, SUB, SUBI, MULT, DIV,
   BEQ, BNE, BGT, BLT, JMP,
   LW, STORE, MOV,
   NOP, HLT, RESET,
   IN, OUT, OUT3, LAB, GOTO
 } Instruction;

 typedef struct AssemblyOperand {
     int value;
     int value2;
     int value3;
 } AssemblyOperand;

 typedef struct AssemblyList{
    struct AssemblyList * next;
    Instruction ins;
    AssemblyOperand op1;
	int pos;
 }* AssemblyList;
 AssemblyList asl;

void asl_initialize();

AssemblyList asl_create(Instruction ins, AssemblyOperand aso);
void asl_insert(AssemblyList as);


 /* Function ail_initialize initializes
  * the assembly instruction list
 */
 void ail_initialize();


/* printa a arvore */
 void ail_print();

 /* Functions ail_create[1-2]? create
  * an instruction to be inserted in
  * the assembly instruction list using
  * an AsmInstKind and zero,
  * one or two Operands
 */
 AsmInstList ail_create(AsmInstKind aik, Operand op1, Operand op2, Operand op3);

 /* Function ail_insert inserts
  * an assembly instruction
  * in the assembly instruction list
 */
 void ail_insert(AsmInstList l);



void codeGen(TreeNode * syntaxTree, char * codefile);

#endif
