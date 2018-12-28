#ifndef _SORT_H_
#define _SORT_H_

#include <stdio.h>
#include "jonewan.h"

typedef void (*FUN_SORT)(int *, int);

#define INSERT_SORT 0
#define CHOOSE_SORT 1
#define SWAP_SORT   2
#define SHELL_SORT  3
#define HEAP_SORT   4
#define QUICK_SORT  5

void insertSort(int *data, int count);
void chooseSort(int *data, int count);
void swapSort(int *data, int count);
void ShellSort(int *data, int count);
void heapSort(int *data, int count);
void quickSort(int *data, int count);
void sort(int *data, int count, int sortType);
#endif
