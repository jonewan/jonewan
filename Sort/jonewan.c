#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>

#include "jonewan.h"

int *makeRandomIntArray(int count, int min, int max){
    int *res = NULL;
    int i = 0;

    res = (int *)calloc(sizeof(int), count);
    srand(time(NULL));
    for(i = 0; i < count; i++){
        res[i] = rand() % (max - min + 1) + min;
    }

    return res;
}

void showIArray(int *arr, int count){
    int i;

    printf("\n");
    for(i = 0; i < count; i++){
        printf("%d ", arr[i]);
    }
    printf("\n");
}

boolean isNumberic(const int ch){
	return isdigit(ch) || isSign(ch) || isDot(ch);
}

const char *skipBlank(const char *str){
	while(0 != *str && isspace(*str))
		str++;

	return str;
}

boolean isSign(const int ch){
	return '+' == ch || '-' == ch;
}

boolean isDot(const int ch){
	return '.' == ch;
}

