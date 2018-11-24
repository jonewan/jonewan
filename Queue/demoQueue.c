#include <stdio.h>

typedef struct POINT{
    int row;
    int col;
    int color;
}POINT;

typedef POINT   USER_TYPE;

#include "queue.h"

int main(void){
    QUEUE   *queueHead = NULL;

    initQueue(&queueHead, 20);

    return 0;
}