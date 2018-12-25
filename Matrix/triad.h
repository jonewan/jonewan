#ifndef _TRIAD_H_
#define _TRIAD_H_

#include "jonewan.h"
#include "userType.h"

typedef struct TRIAD{
	int row;
	int col;
	USER_TYPE value;
} TRIAD;

typedef struct MATRIX{
	TRIAD *triad;
	int count;
	int maxRow;
	int maxCol;
} MATRIX;

void initMatrix(MATRIX  **matrix , int maxRow, int maxCol, int count, TRIAD *triad);
void destoryMatrix(MATRIX **matrix);
void showMatrix(MATRIX matrix);
MATRIX *revengeMatrix(MATRIX matrix);

#endif 