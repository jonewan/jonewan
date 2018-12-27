#include <stdio.h>

int main(void){
	int p[10] = {1,2,3,4,5,6,7,8,9};

	printf("%d, %d", *(p+1), *(1+p));


	return 0;
}
