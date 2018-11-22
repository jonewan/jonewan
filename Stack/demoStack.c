#include <stdio.h>

typedef struct POINT{
	int row;
	int col;
	int color;	
}POINT;

typedef POINT USER_TYPE;
#define STACK_MAX_ELEMENT_COUNT		20


#include "stack.h"

int main(void){
	STACK *pointGroup1 = NULL;

	initStack(&pointGroup1, STACK_MAX_ELEMENT_COUNT);	
	
	destoryStack(&pointGroup1);

	return 0;
}
