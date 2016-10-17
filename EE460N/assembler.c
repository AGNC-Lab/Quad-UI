/*
	Name 1: Mario A Molina
	UTEID 1: mam22696
*/
#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */


FILE* infile = NULL;
FILE* outfile = NULL;

#define MAX_LINE_LENGTH 255
#define OPS 31
#define REG 8

const char * Ops[OPS] = { "add", "and", "br", "brn", "brz", "brp",
"brnz", "brnp", "brzp", "brnzp", "halt", "jmp", "jsr", "jsrr", "ldb",
"ldw", "lea", "nop", "not", "ret", "lshf", "rshfl", "rshfa", "rti",
"stb", "stw", "trap", "xor", ".orig", ".end", ".fill" };

const char * Registers[REG] = { "r0", "r1", "r2", "r3", "r4", "r5",
"r6", "r7"};

enum
{
	DONE, OK, EMPTY_LINE
};

int memorylocation = -1;

/*MY Lables Struct*/
typedef struct {
	char* name;
	int location;
} Lable;

typedef struct {
  int used;
  int size;
  Lable* lables;
} LableTable;

typedef struct{
	char** lables;
	int used;
	int size;
} CalledLableTable;

void lablesExist(LableTable* a, CalledLableTable* b){
	int i, j;
	int exists = 0;
	if(b->used == 0){return;}
	if(a->used == 0){printf("undeclared lable\n"); exit(1);}
	for(i = 0; i < b->used; i++){
		for(j = 0; j < a->used; j++){
			if(strcmp(a->lables[j].name,b->lables[i]) == 0){
				exists = 1;
			}
			if(j == (a->used - 1) && exists == 0){printf("undeclared lable\n"); exit(1);}
		}
		exists = 0;
	};
}

void initCalledLableTable(CalledLableTable* a, int initialSize){
	 a->lables = (char **)malloc(initialSize * sizeof(char*));
 	 a->used = 0;
  	 a->size = initialSize;
}

void insertCalledLable(CalledLableTable* a, char** pString) {
  if(findCalledLable(pString, a) != -1){return;}
  if (a->used == a->size) {
    a->size *= 2;
    a->lables = (char**)realloc(a->lables, a->size * sizeof(char*));
  }

  char* newLable = (char *)malloc(strlen(*pString) * sizeof(char));
  int i;

  for(i = 0; i < strlen(*pString); i++){
  	newLable[i] = *(*pString + i);
  }

  a->lables[a->used++] = newLable;
}

int findCalledLable(char** pString, CalledLableTable* a){
	int i;
	for(i = 0; i < a->used; i++){
		if(strcmp(a->lables[i], *pString) == 0){
			return 1;
		}
	}
	return -1;
}


int findLable(char** pString, LableTable *a){
	int i;
	for(i = 0; i < a->used; i++){
		if(strcmp(a->lables[i].name, *pString) == 0){
			return a->lables[i].location;
		}
	}
	return -1;
}

void initLableTable(LableTable* a, int initialSize) {
  a->lables = (Lable *)malloc(initialSize * sizeof(Lable));
  a->used = 0;
  a->size = initialSize;
}

int validLable(char* pString){
	int l;
	for(l = 0; l < strlen(pString); l++){
  		if(l == 0 && ((*pString) == 'x')){ return 0;}
  		if(l == 0 && (isdigit(*pString)) != 0){ return 0;}	
  		if(isalnum(*(pString + l)) == 0){return 0;}
  }
  return 1;
}

void insertLable(LableTable* a, char** pString, int location) {
  if(findLable(pString, a) != -1){printf("dupicate lable\n"); exit(4);}
  int l;
  for(l = 0; l < strlen(*pString); l++){
  	if(l == 0 && (*(*pString) == 'x')){printf("lable started with x\n"); exit(4);}
  	if(l == 0 && (isdigit(*(*pString)) != 0)){printf("lable started with a numeral\n"); exit(4);}	
  	if(isalnum(*(*pString + l)) == 0){printf("lable contains a non-alphanumeric\n"); exit(4);}
  }

  if (a->used == a->size) {
    a->size *= 2;
    a->lables = (Lable *)realloc(a->lables, a->size * sizeof(Lable));
  }

  char* newLable = (char *)malloc(strlen(*pString) * sizeof(char));
  int i;

  for(i = 0; i < strlen(*pString); i++){
  	newLable[i] = *(*pString + i);
  }

  a->lables[a->used].name = newLable;
  a->lables[a->used++].location = location;
}

void destoryCalledLableTable(CalledLableTable* a){
	int i;
	for(i = 0; i < a->used; i++){
		free(a->lables[i]);
	}
	if(a->used > 0){
		free(a->lables);
	}

	a->lables = NULL;
 	a->used = a->size = 0;
}

void destoryLableTable(LableTable* a){
	int i;
	for(i = 0; i < a->used; i++){
		free(a->lables[i].name);
	}
	if(a->used > 0){
		free(a->lables);
	}

	a->lables = NULL;
 	a->used = a->size = 0;
}


/*MY Lables Struct*/

typedef struct{
  char *lable;
  char *opCode;
  char *arg1;
  char *arg2;
  char *arg3;
  char *arg4;
} Line;

int isOpcode(char* string){
	int i;
	for(i = 0; i < OPS; i++){
		if(strcmp(Ops[i], string) == 0){
			return 1;
		}
	}
	return -1;
}

int isRegister(char* string){
	int i;
	for(i = 0; i < REG; i++){
		if(strcmp(Registers[i], string) == 0){
			return 1;
		}
	}
	return -1;
}

int toNum( char * pStr ){
   char * t_ptr;
   char * orig_pStr;
   int t_length,k;
   int lNum, lNeg = 0;
   long int lNumLong;

   orig_pStr = pStr;
   if( *pStr == '#' )				/* decimal */
   { 
     pStr++;
     if( *pStr == '-' )				/* dec is negative */
     {
       lNeg = 1;
       pStr++;
     }
     t_ptr = pStr;
     t_length = strlen(t_ptr);
     for(k=0;k < t_length;k++)
     {
       if (!isdigit(*t_ptr))
       {
	 printf("Error: invalid decimal operand, %s\n",orig_pStr);
	 exit(4);
       }
       t_ptr++;
     }
     lNum = atoi(pStr);
     if (lNeg)
       lNum = -lNum;
 
     return lNum;
   }
   else if( *pStr == 'x' )	/* hex     */
   {
     pStr++;
     if( *pStr == '-' )				/* hex is negative */
     {
       lNeg = 1;
       pStr++;
     }
     t_ptr = pStr;
     t_length = strlen(t_ptr);
     for(k=0;k < t_length;k++)
     {
       if (!isxdigit(*t_ptr))
       {
	 printf("Error: invalid hex operand, %s\n",orig_pStr);
	 exit(4);
       }
       t_ptr++;
     }
     lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
     lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
     if( lNeg )
       lNum = -lNum;
     return lNum;
   }
   else
   {
	printf( "Error: invalid operand, %s\n", orig_pStr);
	exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
   }
}

int checkSize(int number, int min, int max){
	if(number > max){return 0;}
	if(number < min){return 0;}
	return 1;	
}

int fixOffset(int offset, int min, int max){
	if(checkSize(offset, min, max) == 0){
		printf("invalid lable - too far\n");
		exit(4);
	}
	if(offset < 0){
		offset = ((max + 1)*2) + offset;
	}

	return offset;
}

int checkOp(Line* program){
  /*	"add", "and", "br", "brn", "brz", "brp",
"brnz", "brnp", "brzp", "brnzp", "halt", "jmp", "jsr", "jsrr", "ldb",
"ldw", "lea", "nop", "not", "ret", "lshf", "rshfl", "rshfa", "rti",
"stb", "stw", "trap", "xor", ".orig", ".end", ".fill" };*/

	if(strcmp(program->opCode, ".orig") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
			if(checkSize(toNum(program->arg1),0,65536) == 0 || (toNum(program->arg1)%2 == 1)){ printf("invalid amount\n"); exit(3);}
			memorylocation = toNum(program->arg1) - 2;
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}
	if(strcmp(program->opCode, "add") == 0){
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 != '\0') && (program->arg3 != '\0')){
			if(isRegister(program->arg1) == -1){printf("not a register\n"); exit(4);}
			if(isRegister(program->arg2) == -1){printf("not a register\n"); exit(4);}
			if(isRegister(program->arg3) == -1){
				if(checkSize(toNum(program->arg3),-16,15) == 0){if(*(program->arg3) != 'r'){ printf("invalid amount\n");  exit(3);} 
					else {printf("invalid operand\n");  exit(4);}
				}
			}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	 }
	if(strcmp(program->opCode, "and") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 != '\0') && (program->arg3 != '\0')){
			if(isRegister(program->arg1) == -1){printf("not a register\n"); exit(4);}
			if(isRegister(program->arg2) == -1){printf("not a register\n"); exit(4);}
			if(isRegister(program->arg3) == -1){
				if(checkSize(toNum(program->arg3),-16,15) == 0){if(*(program->arg3) != 'r'){ printf("invalid amount\n");  exit(3);} 
					else {printf("invalid operand\n");  exit(4);}
				}
			}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}	
	if(strcmp(program->opCode, "br") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
			if(validLable(program->arg1) == 0){
				printf("must be a lable\n"); exit(4);
				if(checkSize(toNum(program->arg1),-256,255) == 0){ printf("invalid amount\n"); exit(3);}
			} else { return 1;}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}	
	if(strcmp(program->opCode, "brn") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
			if(validLable(program->arg1) == 0){
				printf("must be a lable\n"); exit(4);
				if(checkSize(toNum(program->arg1),-256,255) == 0){ printf("invalid amount\n"); exit(3);}
			} else { return 1;}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}	
	if(strcmp(program->opCode, "brz") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
			if(validLable(program->arg1) == 0){
				printf("must be a lable\n"); exit(4);
				if(checkSize(toNum(program->arg1),-256,255) == 0){ printf("invalid amount\n"); exit(3);}
			} else { return 1;}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}	
	if(strcmp(program->opCode, "brp") == 0){
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
			if(validLable(program->arg1) == 0){
				printf("must be a lable\n"); exit(4);
				if(checkSize(toNum(program->arg1),-256,255) == 0){ printf("invalid amount\n"); exit(3);}
			} else { return 1;}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	 }	
	if(strcmp(program->opCode, "brnz") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
			if(validLable(program->arg1) == 0){
				printf("must be a lable\n"); exit(4);
				if(checkSize(toNum(program->arg1),-256,255) == 0){ printf("invalid amount\n"); exit(3);}
			} else { return 1;}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}	
	if(strcmp(program->opCode, "brnp") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
			if(validLable(program->arg1) == 0){
				printf("must be a lable\n"); exit(4);
				if(checkSize(toNum(program->arg1),-256,255) == 0){ printf("invalid amount\n"); exit(3);}
			} else { return 1;}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}	
	if(strcmp(program->opCode, "brzp") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
			if(validLable(program->arg1) == 0){
				printf("must be a lable\n"); exit(4);
				if(checkSize(toNum(program->arg1),-256,255) == 0){ printf("invalid amount\n"); exit(3);}
			} else { return 1;}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}
	if(strcmp(program->opCode, "brnzp") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
			if(validLable(program->arg1) == 0){
				printf("must be a lable\n"); exit(4);
				if(checkSize(toNum(program->arg1),-256,255) == 0){ printf("invalid amount\n"); exit(3);}
			} else { return 1;}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}	
	if(strcmp(program->opCode, "halt") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 == '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}	
	if(strcmp(program->opCode, "jmp") == 0){
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
			if(isRegister(program->arg1) == -1){ printf("invalid register\n"); exit(4);}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}	
	if(strcmp(program->opCode, "jsr") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
			if(validLable(program->arg1) == 0){
				printf("must be a lable\n"); exit(4);
				if(checkSize(toNum(program->arg1),-1024,1023) == 0){ printf("invalid amount\n"); exit(3);}
			} else { return 1;}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}	
	if(strcmp(program->opCode, "jsrr") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
			if(isRegister(program->arg1) == -1){ printf("invalid register\n"); exit(4);}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}	
	if(strcmp(program->opCode, "ldb") == 0){
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 != '\0') && (program->arg3 != '\0')){
			if(isRegister(program->arg1) == -1){printf("not a register\n"); exit(4);}
			if(isRegister(program->arg2) == -1){printf("not a register\n"); exit(4);}
			if(checkSize(toNum(program->arg3),-32,31) == 0){ printf("invalid amount\n"); exit(3);}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	 }
	if(strcmp(program->opCode, "ldw") == 0){
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 != '\0') && (program->arg3 != '\0')){
			if(isRegister(program->arg1) == -1){printf("not a register\n"); exit(4);}
			if(isRegister(program->arg2) == -1){printf("not a register\n"); exit(4);}
			if(checkSize(toNum(program->arg3),-32,31) == 0){ printf("invalid amount\n"); exit(3);}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	 }
	if(strcmp(program->opCode, "lea") == 0){
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 != '\0') && (program->arg3 == '\0')){
			if(isRegister(program->arg1) == -1){printf("not a register\n"); exit(4);}
			if(validLable(program->arg2) == 0){
				printf("must be a lable\n"); exit(4); 
				if(checkSize(toNum(program->arg2),-256,255) == 0){ printf("invalid amount\n"); exit(3);}
			} else { return 2;}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}	
	if(strcmp(program->opCode, "nop") == 0){
				if((program->arg4 == '\0') && (program->arg1 == '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
		} else {printf("wrong operands\n"); exit(4); }
		return 0;

	 }	
	if(strcmp(program->opCode, "not") == 0){
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 != '\0') && (program->arg3 == '\0')){
			if(isRegister(program->arg1) == -1){printf("not a register\n"); exit(4);}
			if(isRegister(program->arg2) == -1){printf("not a register\n"); exit(4);}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	 }	
	if(strcmp(program->opCode, "ret") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 == '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}	
	if(strcmp(program->opCode, "lshf") == 0){
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 != '\0') && (program->arg3 != '\0')){
			if(isRegister(program->arg1) == -1){printf("not a register\n"); exit(4);}
			if(isRegister(program->arg2) == -1){printf("not a register\n"); exit(4);}
			if(checkSize(toNum(program->arg3),0,15) == 0){ printf("invalid amount\n"); exit(3);}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	 }
	if(strcmp(program->opCode, "rsfl") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 != '\0') && (program->arg3 != '\0')){
			if(isRegister(program->arg1) == -1){printf("not a register\n"); exit(4);}
			if(isRegister(program->arg2) == -1){printf("not a register\n"); exit(4);}
			if(checkSize(toNum(program->arg3),0,15) == 0){ printf("invalid amount\n"); exit(3);}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}
	if(strcmp(program->opCode, "rshfa") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 != '\0') && (program->arg3 != '\0')){
			if(isRegister(program->arg1) == -1){printf("not a register\n"); exit(4);}
			if(isRegister(program->arg2) == -1){printf("not a register\n"); exit(4);}
			if(checkSize(toNum(program->arg3),0,15) == 0){ printf("invalid amount\n"); exit(3);}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}
	if(strcmp(program->opCode, "rti") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 == '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}	
	if(strcmp(program->opCode, "stb") == 0){
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 != '\0') && (program->arg3 != '\0')){
			if(isRegister(program->arg1) == -1){printf("not a register\n"); exit(4);}
			if(isRegister(program->arg2) == -1){printf("not a register\n"); exit(4);}
			if(checkSize(toNum(program->arg3),-32,31) == 0){ printf("invalid amount\n"); exit(3);}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	 }
	if(strcmp(program->opCode, "stw") == 0){
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 != '\0') && (program->arg3 != '\0')){
			if(isRegister(program->arg1) == -1){printf("not a register\n"); exit(4);}
			if(isRegister(program->arg2) == -1){printf("not a register\n"); exit(4);}
			if(checkSize(toNum(program->arg3),-32,31) == 0){ printf("invalid amount\n"); exit(3);}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	 }
	if(strcmp(program->opCode, "trap") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
			if(checkSize(toNum(program->arg1),0,255) == 0){ printf("invalid amount\n"); exit(3);}
			if(*(program->arg1) != 'x'){printf("not hex\n"); exit(3);}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}
	if(strcmp(program->opCode, "xor") == 0){
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 != '\0') && (program->arg3 != '\0')){
			if(isRegister(program->arg1) == -1){printf("not a register\n"); exit(4);}
			if(isRegister(program->arg2) == -1){printf("not a register\n"); exit(4);}
			if(isRegister(program->arg3) == -1){
				if(checkSize(toNum(program->arg3),-16,15) == 0){if(*(program->arg3) != 'r'){ printf("invalid amount\n");  exit(3);} 
					else {printf("invalid operand\n");  exit(4);}
				}
			}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	 }	
	if(strcmp(program->opCode, ".end") == 0){
	if((program->arg4 == '\0') && (program->arg1 == '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	 }
	if(strcmp(program->opCode, ".fill") == 0){ 
		if((program->arg4 == '\0') && (program->arg1 != '\0') && (program->arg2 == '\0') && (program->arg3 == '\0')){
			if(validLable(program->arg1) == 0){
				if(checkSize(toNum(program->arg1),-32768,65536) == 0){ printf("invalid amount\n"); exit(3);}
			} else { return 1;}
		} else {printf("wrong operands\n"); exit(4); }
		return 0;
	}	
	return 0;
}


/**
*
*
*
*
*
*/


void printOp(Line* program, LableTable* a, CalledLableTable* b){
int lInstr  = 0;

	if(strcmp(program->opCode, ".orig") == 0){ 
		lInstr = toNum(program->arg1);
		memorylocation = toNum(program->arg1) - 2;
	}
	if(strcmp(program->opCode, "add") == 0){
		lInstr = 0x1000;
		program->arg1[0] = program->arg2[0] =  '#';
		lInstr += (toNum(program->arg1)*512 + toNum(program->arg2)*64);
		if(isRegister(program->arg3) == -1){
			if(toNum(program->arg3) < 0){
				lInstr += (32 + (toNum(program->arg3)));
				lInstr += 32;
			} else{
				lInstr += (toNum(program->arg3) + 32);	
			}	
		}else{
			program->arg3[0] =  '#';
			lInstr += toNum(program->arg3);	
		}
	}
	if(strcmp(program->opCode, "and") == 0){ 
		lInstr = 0x5000;
		program->arg1[0] = program->arg2[0] =  '#';
		lInstr += (toNum(program->arg1)*512 + toNum(program->arg2)*64);
		if(isRegister(program->arg3) == -1){
			if(toNum(program->arg3) < 0){
				lInstr += (toNum(program->arg3) + 32);
				lInstr += 32;
			} else{
				lInstr += (toNum(program->arg3) + 32);	
			}	
		}else{
			program->arg3[0] =  '#';
			lInstr += toNum(program->arg3);	
		}
	}	
	if(strcmp(program->opCode, "br") == 0){ 
			lInstr = 0x0E00;
			if(validLable(program->arg1) == 0){	
				if(toNum(program->arg1) < 0){
				lInstr += ((toNum(program->arg1) + 512));
				} else{
				lInstr += (toNum(program->arg1));	
				}	
			} 
			else {lInstr += fixOffset(((findLable(&(program->arg1),a) - (memorylocation + 2))/2), -256, 255);}
	}	
	if(strcmp(program->opCode, "brn") == 0){ 
		lInstr = 0x0800;
			if(validLable(program->arg1) == 0){	
				if(toNum(program->arg1) < 0){
				lInstr += ((toNum(program->arg1) + 512));
				} else{
				lInstr += (toNum(program->arg1));	
				}	
			} 
			else {lInstr += fixOffset(((findLable(&(program->arg1),a) - (memorylocation + 2))/2), -256, 255);}
	}	
	if(strcmp(program->opCode, "brz") == 0){ 
		lInstr = 0x0400;
			if(validLable(program->arg1) == 0){	
				if(toNum(program->arg1) < 0){
				lInstr += ((toNum(program->arg1) + 512));
				} else{
				lInstr += (toNum(program->arg1));	
				}	
			} 
			else {lInstr += fixOffset(((findLable(&(program->arg1),a) - (memorylocation + 2))/2), -256, 255);}
	}	
	if(strcmp(program->opCode, "brp") == 0){
		lInstr = 0x0200;
			if(validLable(program->arg1) == 0){	
				if(toNum(program->arg1) < 0){
				lInstr += ((toNum(program->arg1) + 512));
				} else{
				lInstr += (toNum(program->arg1));	
				}	
			} 
			else {lInstr += fixOffset(((findLable(&(program->arg1),a) - (memorylocation + 2))/2), -256, 255);}
	 }	
	if(strcmp(program->opCode, "brnz") == 0){ 
		lInstr = 0x0c00;
			if(validLable(program->arg1) == 0){	
				if(toNum(program->arg1) < 0){
				lInstr += ((toNum(program->arg1) + 512));
				} else{
				lInstr += (toNum(program->arg1));	
				}	
			} 
			else {lInstr += fixOffset(((findLable(&(program->arg1),a) - (memorylocation + 2))/2), -256, 255);}
	}	
	if(strcmp(program->opCode, "brnp") == 0){ 
		lInstr = 0x0a00;
			if(validLable(program->arg1) == 0){	
				if(toNum(program->arg1) < 0){
				lInstr += ((toNum(program->arg1) + 512));
				} else{
				lInstr += (toNum(program->arg1));	
				}	
			} 
			else {lInstr += fixOffset(((findLable(&(program->arg1),a) - (memorylocation + 2))/2), -256, 255);}
		
	}	
	if(strcmp(program->opCode, "brzp") == 0){ 
		lInstr = 0x0600;
			if(validLable(program->arg1) == 0){	
				if(toNum(program->arg1) < 0){
					lInstr += ((toNum(program->arg1) + 512));
				} else{
				lInstr += (toNum(program->arg1));	
				}	
			} 
			else {lInstr += fixOffset(((findLable(&(program->arg1),a) - (memorylocation + 2))/2), -256, 255);}
		
	}
	if(strcmp(program->opCode, "brnzp") == 0){ 
		lInstr = 0x0E00;
			if(validLable(program->arg1) == 0){	
				if(toNum(program->arg1) < 0){
					lInstr += ((toNum(program->arg1) + 512));
				} else{
				lInstr += (toNum(program->arg1));	
				}	
			} 
			else {lInstr += fixOffset(((findLable(&(program->arg1),a) - (memorylocation + 2))/2), -256, 255);}
		
	}	
	if(strcmp(program->opCode, "halt") == 0){ 
		lInstr = 0xF025;
	}	
	if(strcmp(program->opCode, "jmp") == 0){
		lInstr = 0xc000;
		program->arg1[0] = '#';
		lInstr += (toNum(program->arg1)*64);	
	}	
	if(strcmp(program->opCode, "jsr") == 0){ 
		lInstr = 0x4800;
		if(validLable(program->arg1) == 0){	
				if(toNum(program->arg1) < 0){
					lInstr += ((toNum(program->arg1) + 2048));
				} else{
					lInstr += (toNum(program->arg1));	
				}	
			} 
			else {lInstr += fixOffset(((findLable(&(program->arg1),a) - (memorylocation + 2))/2),-1024, 1023);}
		
	}	
	if(strcmp(program->opCode, "jsrr") == 0){ 
		lInstr = 0x4000;
		program->arg1[0] = '#';
		lInstr += (toNum(program->arg1)*64);
		
	}	
	if(strcmp(program->opCode, "ldb") == 0){
		lInstr = 0x2000;
		program->arg1[0] = program->arg2[0] =  '#';
		lInstr += (toNum(program->arg1)*512 + toNum(program->arg2)*64);
		if(toNum(program->arg3) < 0){
			lInstr += ((toNum(program->arg3) + 64));
		} else{
			lInstr += (toNum(program->arg3));	
		}			
	 }
	if(strcmp(program->opCode, "ldw") == 0){
		lInstr = 0x6000;
		program->arg1[0] = program->arg2[0] =  '#';
		lInstr += (toNum(program->arg1)*512 + toNum(program->arg2)*64);
		if(toNum(program->arg3) < 0){
			lInstr += ((toNum(program->arg3)+ 64));
		} else{
			lInstr += (toNum(program->arg3));	
		}	
		
	 }
	if(strcmp(program->opCode, "lea") == 0){
		lInstr = 0xE000;
		program->arg1[0] = '#';
		lInstr += (toNum(program->arg1)*512);
		if(validLable(program->arg2) == 0){	
				if(toNum(program->arg2) < 0){
				lInstr += ((toNum(program->arg2) + 512));
				} else{
				lInstr += (toNum(program->arg2));	
				}	
			} 
			else {lInstr += fixOffset(((findLable(&(program->arg2),a) - (memorylocation + 2))/2), -256, 255);}
	}	

	if(strcmp(program->opCode, "nop") == 0){
		lInstr = 0x0000;
	 }	
	if(strcmp(program->opCode, "not") == 0){
		lInstr = 0x9000;
		program->arg1[0] = program->arg2[0] =  '#';
		lInstr += (toNum(program->arg1)*512 + toNum(program->arg2)*64);
		lInstr += 63;
		
	 }	
	if(strcmp(program->opCode, "ret") == 0){ 
		lInstr = 0xc1c0;		
	}	
	if(strcmp(program->opCode, "lshf") == 0){
		lInstr = 0xD000;
		program->arg1[0] = program->arg2[0] =  '#';
		lInstr += (toNum(program->arg1)*512 + toNum(program->arg2)*64);
		lInstr += (toNum(program->arg3));				
	 }
	if(strcmp(program->opCode, "rshfl") == 0){ 
		lInstr = 0xD000;
		program->arg1[0] = program->arg2[0] =  '#';
		lInstr += (toNum(program->arg1)*512 + toNum(program->arg2)*64);
		lInstr += (toNum(program->arg3));	
		lInstr += 16;
	}
	if(strcmp(program->opCode, "rshfa") == 0){ 
		lInstr = 0xD000;
		program->arg1[0] = program->arg2[0] =  '#';
		lInstr += (toNum(program->arg1)*512 + toNum(program->arg2)*64);
		lInstr += (toNum(program->arg3));	
		lInstr += 48;		
	}
	if(strcmp(program->opCode, "rti") == 0){ 
		lInstr = 0x8000;		
	}	
	if(strcmp(program->opCode, "stb") == 0){
		lInstr = 0x3000;
		program->arg1[0] = program->arg2[0] =  '#';
		lInstr += (toNum(program->arg1)*512 + toNum(program->arg2)*64);
		if(toNum(program->arg3) < 0){
			lInstr += (toNum(program->arg3) + 64);
		} else{
			lInstr += (toNum(program->arg3));	
		}	
	}
	if(strcmp(program->opCode, "stw") == 0){
		lInstr = 0x7000;
		program->arg1[0] = program->arg2[0] =  '#';
		lInstr += (toNum(program->arg1)*512 + toNum(program->arg2)*64);
		if(toNum(program->arg3) < 0){
			lInstr += (toNum(program->arg3) + 64);
		} else{
			lInstr += (toNum(program->arg3));	
		}	
	 }
	if(strcmp(program->opCode, "trap") == 0){ 
		lInstr = 0xF000;
		lInstr += toNum(program->arg1);
		
	}
	if(strcmp(program->opCode, "xor") == 0){
		lInstr = 0x9000;
		program->arg1[0] = program->arg2[0] =  '#';
		lInstr += (toNum(program->arg1)*512 + toNum(program->arg2)*64);
		if(isRegister(program->arg3) == -1){
			if(toNum(program->arg3) < 0){
				lInstr += (32 + (toNum(program->arg3)));
				lInstr += 32;
			} else{
				lInstr += (toNum(program->arg3) + 32);	
			}	
		}else{
			program->arg3[0] =  '#';
			lInstr += toNum(program->arg3);	
		}
	 }	
	if(strcmp(program->opCode, ".end") == 0){
		return;
	 }
	if(strcmp(program->opCode, ".fill") == 0){ 
		if(validLable(program->arg1) == 0){	
				if(toNum(program->arg1) < 0){
				lInstr = ((toNum(program->arg1) + 65536));
				} else{
				lInstr = (toNum(program->arg1));	
				}	
		} else {lInstr = findLable(&(program->arg1),a);}
	}	


fprintf( outfile, "0x%.4X\n", lInstr );
}

int readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char
	** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4
	)
	{
	   char * lRet, * lPtr;
	   int i;

	   if( !fgets( pLine, MAX_LINE_LENGTH, pInfile ) ){return( DONE );}


	   /* convert entire line to lowercase */
	   for( i = 0; i < strlen( pLine ); i++ ){
		pLine[i] = tolower( pLine[i] );
	   }

	   *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = NULL;

	   /* ignore the comments */
	   lPtr = pLine;

	   while( *lPtr != ';' && *lPtr != '\0' &&
	   *lPtr != '\n' ) { lPtr++; }

	   *lPtr = '\0';
	   if( !(lPtr = strtok( pLine, "\t\n ," ) ) ) {return( EMPTY_LINE );}

	   if( isOpcode( lPtr ) == -1) /* found a label */
	   {
		*pLabel = lPtr;
		if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
	   }
	   
           *pOpcode = lPtr;

	   if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
	   
           *pArg1 = lPtr;
	   
           if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	   *pArg2 = lPtr;
	   if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	   *pArg3 = lPtr;

	   if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	   *pArg4 = lPtr;

	   return( OK );
	}

int main(int argc, char* argv[]) {

     /*checks arguments*/
     if(argc != 3){
     	printf("Must have 3 arguments not %i\n", argc);
     	exit(4);
     }

     /* open the source file */
     infile = fopen(argv[1], "r");
     outfile = fopen(argv[2], "w");
		 
     if (!infile) {
       printf("Error: Cannot open file %s\n", argv[1]);
       exit(4);
		 }
     if (!outfile) {
       printf("Error: Cannot open file %s\n", argv[2]);
       exit(4);
     }

     /* Do stuff with files */
     int enumeratorValue = OK;
     int foundEnd = 0;
     Line currentLine;
     LableTable myTable;
     CalledLableTable myCalledTable;
     initLableTable(&myTable, 5);
     initCalledLableTable(&myCalledTable, 5);
     char* linePointer = (char*) malloc(sizeof(char)*255);

     while(enumeratorValue != DONE && foundEnd == 0){
		enumeratorValue = readAndParse(infile, linePointer, &currentLine.lable, 
     	&currentLine.opCode, &currentLine.arg1, &currentLine.arg2,
     	&currentLine.arg3, &currentLine.arg4);

		if(enumeratorValue == OK){
			if((currentLine.opCode)){
				if(strcmp(currentLine.opCode, ".end") == 0) {foundEnd = 1;}
				if(isOpcode(currentLine.opCode) == -1){printf("invalid OpCode\n"); exit(2);}
				int state = checkOp(&currentLine);
				if(state == 1){insertCalledLable(&myCalledTable,&currentLine.arg1);}
				if(state == 2){insertCalledLable(&myCalledTable,&currentLine.arg2);}
			} else {printf("invalid OpCode\n"); exit(2);}
			if((currentLine.lable)){
			  if(strcmp(currentLine.opCode, ".orig") == 0){printf("invalid place to put lable\n"); exit(4);}	
			  if(strcmp(currentLine.opCode, ".end") == 0){printf("invalid place to put lable\n"); exit(4);}
			insertLable(&myTable, &currentLine.lable, memorylocation);}
	     	if((currentLine.lable != NULL) || (currentLine.opCode != NULL)){memorylocation += 2;}
	     	if(memorylocation == -1){printf(".ORIG should be first\n"); exit(4);}
     	}

     }
     lablesExist(&myTable, &myCalledTable);
     rewind(infile);
     enumeratorValue = OK;
     foundEnd = 0;

     while(enumeratorValue != DONE && foundEnd == 0){
		enumeratorValue = readAndParse(infile, linePointer, &currentLine.lable, 
     	&currentLine.opCode, &currentLine.arg1, &currentLine.arg2,
     	&currentLine.arg3, &currentLine.arg4);

		if(enumeratorValue == OK){
			if((currentLine.opCode)){ 
				if(strcmp(currentLine.opCode, ".end") == 0) {foundEnd = 1;}
				printOp(&currentLine, &myTable, &myCalledTable);
			} 
		}
		if((currentLine.lable != NULL) || (currentLine.opCode != NULL)){memorylocation += 2;}

	 }


     fclose(infile);
     fclose(outfile);
     destoryCalledLableTable(&myCalledTable);
     destoryLableTable(&myTable);

     /* Do stuff with files */
     /*
     prgName   = argv[0];
     iFileName = argv[1];
     oFileName = argv[2];

     printf("Arguments = '%i'\n", argc);
     printf("program name = '%s'\n", prgName);
     printf("input file name = '%s'\n", iFileName);
     printf("output file name = '%s'\n", oFileName);
     */
}