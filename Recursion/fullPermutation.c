//fileName:	fullPermutation.c
//author:	jonewan
//date:		2018/11/21


#include <stdio.h>

#define MAX_ARRAY_COUNT		16
#define PERMUTATION_COUNT	4

char alpha[] = "0123456789ABCDEF";

void showPermutation(char *Permutation);
void fullPermutation(int *indexArr, char *Permutation, int index);

void fullPermutation(int *indexArr, char *Permutation, int index){
	int count = 0;

	if(index >= PERMUTATION_COUNT)
		showPermutation(Permutation);
	else{
		for(count = 0; count < MAX_ARRAY_COUNT; count++){
			//if(indexArr[count] == 0)
			{
				indexArr[count] = 1;
				Permutation[index] = alpha[count];
				fullPermutation(indexArr, Permutation, index+1);
				indexArr[count] = 0;
			}
		}
	}

}

void showPermutation(char *Permutation){
	int i = 0;
	static unsigned int callCount = 1;

	printf("第%d种排列结果：\n", callCount++);
	puts(Permutation);
	puts("\n");
}


int main(void){
	int indexArr[MAX_ARRAY_COUNT] = {0};
	char Permutation[PERMUTATION_COUNT];

	fullPermutation(indexArr, Permutation, 0);

	return 0;
}
