#include <stdio.h>
#include <malloc.h>

#include "jonewan.h"
#include "sort.h"

extern FUN_SORT sort_name[];

int main(void){
    int *array = NULL;
    int count, min, max;

    printf("Please input count, min & max:");
    scanf("%d%d%d", &count, &min, &max);
    array = makeRandomIntArray(count, min, max);
    printf("orgin dara: ");
    showIArray(array, count);
    sort(array, count, QUICK_SORT);
//    quickSort(array, count);
    printf("After sort");
    showIArray(array, count);

    free(array);

    return 0;
}

