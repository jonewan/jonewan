#include <stdio.h>
#include <malloc.h>

typedef unsigned int boolean;

#define TRUE	1
#define FALSE	0

boolean isArrayGrow(int *arr, int index);

boolean isArrayGrow(int *arr, int index){
	static boolean ok = TRUE;
	static int recursionCount = 1;
	
	printf("the %d th recursion\n", recursionCount++);
	if(index <= 0)
		ok = TRUE;
	else if(ok == TRUE){
		if(arr[index] > arr[index-1])
			ok = isArrayGrow(arr, index -1);
		else
			ok = FALSE;
	}
	
	return ok;
}

int main(){
	int *a = NULL;
	int arrCount = 0;
	int i = 0;

	printf("input array count:");
	scanf("%d", &arrCount);
	a = (int *)malloc(sizeof(int) * arrCount);
	for(i = 0; i < arrCount; i++){
		printf("input the %d th num:", i+1);
		scanf("%d", &a[i]);
	}

	if(TRUE == isArrayGrow(a, arrCount-1))
		puts("array is grow!\n");
	else
		puts("array not grow!!!\n");


	return 0;
}
