//fileName:	polynomial.c
//author:	jonewan
//date:		2018/11/20

#include <stdio.h>
#include <math.h>

typedef struct POLYNOMIAL{
	double coefficient;//系数
	int power;//指数
} POLYNOMIAL;

typedef POLYNOMIAL USER_TYPE;

#include "linear.h"

#define POLYNOMIAL_ELEMENT_MAX_COUNT	20

void initPolynomial(LINEAR *head);
void showPolynomial(LINEAR head);

void showPolynomial(LINEAR head){
	int i;
	int count;
	POLYNOMIAL value;

	count = getLinearElementCount(head);
	for(i = 0; i < count; i++){
		getLinearElementAt(&value, head, i);
		if(0 == i){
			printf("%.4lf", value.coefficient);
		}
		else{
			if(value.coefficient > 1e-6)
				printf("+");
			printf("%.4lf", value.coefficient);
		}
		if(value.power != 0){
			printf("x");
			if(value.power != 1)
				printf("^%d",value.power);
		}
	}

}


/*用户从键盘输入多项式的每一项，并存储到线性表中；
 *若用户输入的向，其系数为0，表示输入结束
 *不允许用户输入的项的个数超过线性表最大空间
 */
void initPolynomial(LINEAR *head){
	double coefficient;//系数
	int power;//指数
	int maxRoom;
	int count = 0;
	USER_TYPE value;

	maxRoom = getLinearMaxRoom(*head);
	printf("请输入一项（由系数和指数组成，且，系数为0结束输入）：");
	scanf("%lf%d", &coefficient, &power);

	while(count < maxRoom && fabs(coefficient) > 1e-6){
		value.coefficient = coefficient;
		value.power = power;
		
		appendLinear(head, value);
		count++;
		printf("请输入一项（由系数和指数组成，且，系数为0结束输入）：");
		scanf("%lf%d", &coefficient, &power);
	}
	puts("\n*********\n");
}


int main(void){
	LINEAR *polyPoint = NULL;

	printf("&polyPoint = %p\npolyPoint = %p\n", &polyPoint, polyPoint);
	printf("\n..............\n");
	initLinear(&polyPoint, POLYNOMIAL_ELEMENT_MAX_COUNT);
	printf("&polyPoint = %p\npolyPoint = %p\npolyPoint->data = %p\n", &polyPoint, polyPoint, polyPoint->data);
	initPolynomial(polyPoint);
//	showPolynomial(*polyPoint);	

	puts("\n*********\n");
	printf("&polyPoint = %p\npolyPoint = %p\npolyPoint->data = %p\n", &polyPoint, polyPoint, polyPoint->data);
	destoryLinear(&polyPoint);
	
	printf("\n..............\n");

	return 0;
}

