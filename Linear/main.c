//fileName:	main.c
//author:	jonewan
//date:		2018/11/19	

#include <stdio.h>

typedef int USER_TYPE;

#include "./linear.h"

int cmpData(USER_TYPE one, USER_TYPE another);
void showData(LINEAR head);

void showData(LINEAR head){
	int i = 0;
	int elementCount = getLinearElementCount(head);
	USER_TYPE value;

	for(i = 0; i < elementCount; i++){
		getLinearElementAt(&value, head, i);
		printf("%d ", value);
	}
}

int cmpData(USER_TYPE one, USER_TYPE another){
	return one - another;
}

int main(void){
	LINEAR *linear = NULL;
	USER_TYPE value;
	int i;

	initLinear(&linear, 100);
	for(i = 0; i < 6; i++){
		printf("请输入第%d个（共6个）数据：", i+1);
		scanf("%d", &value);
		appendLinear(linear, value);
	}	
	showData(*linear);

	destoryLinear(&linear);

	return 0;
}
