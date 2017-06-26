/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "symtab.h"

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

#define MAX_SCOPE 1000

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

static Scope scopes[MAX_SCOPE];
static int nScope = 0;
static Scope scopeStack[MAX_SCOPE];
static int nScopeStack = 0;
static int location[MAX_SCOPE];

static int escopoMemoria = 0;

Scope sc_top( void )
{ return scopeStack[nScopeStack - 1];
}

void sc_pop( void )
{
  //printf("pop %s\n", sc_top()->funcName);
  --nScopeStack;
}

int addLocation( void )
{
  //return location[nScopeStack - 1]++;
  return -1;
}

int declaraVariavel ( void ){
	escopoMemoria++;
	return escopoMemoria;
}

int declaraVetor ( int tam ){
	int varVet = escopoMemoria +1;
	escopoMemoria = escopoMemoria + tam;
	return varVet;
}


void sc_push( Scope scope )
{ scopeStack[nScopeStack] = scope;
  location[nScopeStack++] = 0;
  //printf("push %s\n", scope->funcName);
}

Scope sc_create(char *funcName)
{ Scope newScope;

  newScope = (Scope) malloc(sizeof(struct ScopeRec));
  newScope->funcName = funcName;
  newScope->nestedLevel = nScopeStack;
  newScope->parent = sc_top();

  scopes[nScope++] = newScope;

  return newScope;
}

BucketList st_bucket( char * name )
{ int h = hash(name);
  Scope sc = sc_top();
  while(sc) {
    BucketList l = sc->hashTable[h];
    while ((l != NULL) && (strcmp(name,l->name) != 0))
      l = l->next;
    if (l != NULL) return l;
    sc = sc->parent;
  }
  return NULL;
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, char * tipo, int lineno, int loc, TreeNode * treeNode ) {
	int h = hash(name);
	Scope top = sc_top();
  	BucketList l = top->hashTable[h];

  	while ((l != NULL) && (strcmp(name,l->name) != 0))
    		l = l->next;
  	if (l == NULL) { /* variable not yet in table */
  		l = (BucketList) malloc(sizeof(struct BucketListRec));
    		l->name = name;
		l->treeNode = treeNode;
		l->tipo = tipo;
    		l->lines = (LineList) malloc(sizeof(struct LineListRec));
	    	l->lines->lineno = lineno;
	    	l->memloc = loc;
	    	l->lines->next = NULL;
	    	l->next = top->hashTable[h];
	   	top->hashTable[h] = l;
	}
} /* st_insert */

int decimal_binario (int numero){
	return numero;
}

int search_pos_var (int var){
	int i;
	int j;
	for (i = 0; i < nScope; ++i) {
		Scope scope = scopes[i];
		BucketList * hashTable = scope->hashTable;
		for (j = 0; j < SIZE; ++j) {
			if (hashTable[j] != NULL) {
				BucketList l = hashTable[j];
					TreeNode *node = l->treeNode;

					while (l != NULL) {
						LineList t = l->lines;
						if (j == var){
							//achou a variavel
							return l->memloc;
						}
						l = l->next;
					}
			}
		}
	}
	return -2;
}

int st_lookup ( char * name )
{ BucketList l = st_bucket(name);
  if (l != NULL) return l->memloc;
  return -1;
}

int st_lookup_top (char * name)
{ int h = hash(name);
  Scope sc = sc_top();
  while(sc) {
    BucketList l = sc->hashTable[h];
    while ((l != NULL) && (strcmp(name,l->name) != 0))
      l = l->next;
    if (l != NULL) return l->memloc;
    break;
  }
  return -1;
}

int busca_funcao_escopo_global(char * name){
	Scope scope = scopes[0];
	BucketList * hashTable = scope->hashTable;
	int j;
	for (j = 0; j < SIZE; ++j) {
		if (hashTable[j] != NULL) {
			BucketList l = hashTable[j];
			TreeNode *node = l->treeNode;

			while (l != NULL) {
				LineList t = l->lines;
				if(strcmp(l->name, name) == 0)
					if(strcmp(l->treeNode->attr.type, "Função") == 0)
						return 1;
				l = l->next;
			}
   		}
  	}
	return -1;
}

int st_func_top(char * name){
  int h = hash(name);
  if(h == NULL)
	  return -1;
  return h;
}

int st_lookup_func ( char * name )
{ BucketList l = st_bucket(name);
  if (l == NULL){
    return -1;
  }
  if (strcmp(l->tipo, "Void") == 0){
    return -1;
  }
  return 1;
}

int st_add_lineno(char * name, int lineno) {
	BucketList l = st_bucket(name);
  	LineList ll = l->lines;
  	while (ll->next != NULL)
		ll = ll->next;
	if (ll->lineno != lineno) {
	 	ll->next = (LineList) malloc(sizeof(struct LineListRec));
	  	ll->next->lineno = lineno;
	  	ll->next->next = NULL;
	}
}

int cgen_search_top(char * name){
  int h = hash(name);
  return h;
}

/* Função para retornar o nome do tipo */
char* idTypeToStr( ExpType type ) {
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

void printSymTabRows(BucketList *hashTable, FILE *listing) {
	int j;

	for (j = 0; j < SIZE; ++j) {
		if (hashTable[j] != NULL) {
			BucketList l = hashTable[j];
      			TreeNode *node = l->treeNode;

      			while (l != NULL) {
					LineList t = l->lines;
					fprintf(listing,"%-8d ",j);
					fprintf(listing,"%-17s ",l->name);
					fprintf(listing,"%-7s   ",l->tipo);
					fprintf(listing,"%-10s ",l->treeNode->attr.type);
						while (t != NULL) {
						if (t->lineno != -1)
							fprintf(listing,"%4d ",t->lineno);
							t = t->next;
						}
						if((strcmp(l->treeNode->attr.type, "Variável") == 0) || (strcmp(l->treeNode->attr.type, "Vetor") == 0)){
							fprintf(listing,"%13d",l->memloc);
						}
						
					
						fprintf(listing,"\n");
						l = l->next;
     			}
   		}
  	}
	
}



void printSymTab(FILE * listing) {
	int i;

	for (i = 0; i < nScope; ++i) {
		Scope scope = scopes[i];
		BucketList * hashTable = scope->hashTable;

		if (i == 0) { // Escopo global
			fprintf(listing, "<Escopo Global>\n");
		} else {
			fprintf(listing, "Escopo: <%s>\n", scope->funcName);
		}

		fprintf(listing,"Entrada  Nome da variavel  Tipo ID   Tipo dados  Numero das linhas   PosMemoria\n");
  		fprintf(listing,"-----  ----------------  --------  ----------  -----------------   ----\n");

		printSymTabRows(hashTable, listing);
		fputc('\n', listing);
	}
} /* printSymTab */
