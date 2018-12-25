#include <stdio.h>
#include <malloc.h>

#include "userType.h"
#include "jonewan.h"
#include "triad.h"

void initMatrix(MATRIX  **matrix , int maxRow, int maxCol, int count, TRIAD *triad){
    if(NULL == matrix || NULL != *matrix || NULL == triad
        || maxRow <= 0 || maxCol <= 0 
        || count <= 0){
            return ;
    }

    MATRIX *m = (MATRIX *)calloc(sizeof(MATRIX), 1);
    *matrix = m;
    m->maxRow = maxRow;
    m->maxCol = maxCol;
    m->count = count;
    m->triad = (TRIAD *)calloc(sizeof(TRIAD), count);

    int i = 0;

    for(i = 0; i < count ; i++){
        m->triad[i].row = triad[i].row;
        m->triad[i].col = triad[i].col;
        m->triad[i].value = triad[i].value;
    }
}

void destoryMatrix(MATRIX **matrix){
    if(NULL == matrix || NULL == *matrix){
        return;
    }
    free((*matrix)->triad);
    free(*matrix);
    matrix = NULL;
}

void showMatrix(MATRIX matrix){
    int i, j;
    int index = 0;

    printf("矩阵如下:\n");
    for(i = 0; i < matrix.maxRow; i++){
        for(j = 0; j < matrix.maxCol; j++){
            if(i == matrix.triad[index].row && j == matrix.triad[index].col){
                printf("%4d", matrix.triad[index].value);;
                index++;
            }else{
                printf("%4d", 0);
            }
        }
        printf("\n");
    }
}

MATRIX *revengeMatrix(MATRIX matrix){
    MATRIX *result = (MATRIX *)calloc(sizeof(MATRIX), 1);

    result->triad = (TRIAD*)calloc(sizeof(TRIAD), matrix.count);
    result->maxRow = matrix.maxCol;
    result->maxCol = matrix.maxRow;
    result->count = matrix.count;

    int *index = (int *)calloc(sizeof(int), matrix.count + 1);
    int i;
    TRIAD triad;
    int  t;

    for(i = 0; i < matrix.count; i++){
        index[matrix.triad[i].col + 1]++;
    }

    for(i = 1; i < matrix.maxCol+1; i++){
        index[i] += index[i-1];
    }

    for(i = 0; i < matrix.count; i++){
        triad = matrix.triad[i];
        t = index[triad.col];
        result->triad[t].row = triad.col;
	    result->triad[t].col = triad.row;
	    result->triad[t].value = triad.value;
        index[triad.col]++;
    }
    free(index);

    return result;
}
