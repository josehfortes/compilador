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
                 VarAsgK, VecAsgK,
                 InputK, OutputK,
                 FunctionParameterK, FunctionCallK, FunctionReturnK, LabK,
                 GotoK,
				 If_FK
 } AsmInstKind;

 typedef struct Operand {
   OperandKind kind;
   int value;
	 int value2;
	 int pos;
	 int tempValue;
	 int tempValue2;
	OperandKind vecType;
 } Operand;

 typedef struct AsmInstList {
     struct AsmInstList * next;
     AsmInstKind aik;
     Operand op1;
     Operand op2;
	 int Temp;
 } * AsmInstList;

 AsmInstList ail;

 typedef enum {
   ADD, ADDI, SUB, SUBI, MULT, DIV,
   BEQ, BNE, BGT, BLT, JMP,
   LW, STORE, MOV,
   NOP, HLT, RESET,
   IN, OUT, OUT3
 } Instruction;

 typedef struct AssemblyOperand {
     int value;
     int flagValue;
     int value2;
     int flagValue2;
     int value3;
     int flagValue3;
 } AssemblyOperand;

 typedef struct AssemblyList{
    struct AssemblyList * next;
    Instruction ins;
    AssemblyOperand op1;
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
 AsmInstList ail_create(AsmInstKind aik);
 AsmInstList ail_create1(AsmInstKind aik, Operand op1);
 AsmInstList ail_create2(AsmInstKind aik, Operand op1, Operand op2);

 /* Function ail_insert inserts
  * an assembly instruction
  * in the assembly instruction list
 */
 void ail_insert(AsmInstList l);



void codeGen(TreeNode * syntaxTree, char * codefile);

#endif
