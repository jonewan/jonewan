#include <stdio.h>
#include <malloc.h>

#include "triad.h"

void getTriad(MATRIX **matrix);

void getTriad(MATRIX **matrix){
    int count;
    int maxRow, maxCol;
    int value;
    int i;
    int row, col;

    printf("输入矩阵行阶和矩阵列阶：");
    scanf("%d%d", &maxRow, &maxCol);
    printf("请输入有效元素个数:");
    scanf("%d", &count);

    TRIAD *triad = (TRIAD *)calloc(sizeof(TRIAD), count);
    printf("请输入%d个数据：\n", count);
    for(i = 0; i < count; i++){
        printf("第%d/%d个:", i+1, count);
        scanf("%d%d%d", &row, &col, &value);
        triad[i].row = row;
        triad[i].col = col;
        triad[i].value = value;
    }

    initMatrix(matrix, maxRow, maxCol, count, triad);
    free(triad);
}

int main(void){
    MATRIX *matrix = NULL;
    MATRIX* revMatrix = NULL;

    getTriad(&matrix);
    showMatrix(*matrix);
    revMatrix = revengeMatrix(*matrix);
    showMatrix(*revMatrix);
    destoryMatrix(&matrix);
    destoryMatrix(&revMatrix);


    return 0;
}