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
void sortPolynomialByPower(LINEAR head);
LINEAR *polynomialAdd(LINEAR p1, LINEAR p2);

/*实现多项式p1和多项式p2的求和，结果（多项式）应该是一个新的多项式，并通过函数实现；
 *1、多项式p1和多项式p2在运算过程中，不做任何更改；
 *2、产生的结果多项式应该是申请的新的线性表；
 *3、新线性表的最大空间应该是p1的空间和p2的空间的和值；
 */
LINEAR *polynomialAdd(LINEAR p1, LINEAR p2){
	LINEAR *result = NULL;
	int indexP1 = 0;
	int indexP2 = 0;
	POLYNOMIAL	vP1;
	POLYNOMIAL	vP2;
	int countP1;
	int countP2;

	initLinear(&result, getLinearMaxRoom(p1) + getLinearMaxRoom(p2));
	/*将两个多项式，按指数从高到低的顺序，存储到目标多项式中；
	 *若指数相同，则系数相加；
	 *	若系数相加的结果为0， 则，不应将该项写入多项式中；
	 *
	 *	分别从p1和p2中取出当前项；
	 *	若p1和p2的指数不同，将较大的指数的项，append到目标中
	 *	较大的指数所对应的多项式的下标要+1，取出新的一项进行比较
	 *	当两个多项式中任意一个，处理完所有项时，这个动作应该停止
	 *	最后将那个没处理完的多项式全部添加到目标的结尾
	 */
	countP1 = getLinearElementCount(p1);
	countP2 = getLinearElementCount(p2);

	while(indexP1 < countP1 && indexP2 < countP2){
		getLinearElementAt(&vP1, p1, indexP1);
		getLinearElementAt(&vP2, p2, indexP2);
		if(vP1.power > vP2.power){
			appendLinear(result, vP1);
			indexP1++;
		}
		else if(vP1.power < vP2.power){
			appendLinear(result, vP2);
			indexP2++;
		}
		else{
			vP1.coefficient += vP2.coefficient;
			if(fabs(vP1.coefficient) > 1e-6)
				appendLinear(result, vP1);
			indexP1++;
			indexP2++;
		}
	}
	for(; indexP1 < countP1; indexP1++){
		getLinearElementAt(&vP1, p1, indexP1);
		appendLinear(result, vP1);
	}
	
	for(; indexP2 < countP2; indexP2++){
		getLinearElementAt(&vP2, p2, indexP2);
		appendLinear(result, vP2);
	}
		

	return result;
}

void sortPolynomialByPower(LINEAR head){
	int i, j;
	USER_TYPE temp;
	USER_TYPE vi, vj;
	int cnt = getLinearElementCount(head);

	for(i = 0; i < cnt; i++){
		getLinearElementAt(&vi, head, i);
		for(j = i+1; j < cnt; j++){
			getLinearElementAt(&vj, head, j);
			if(vi.power < vj.power){
				temp = vi;
				vi = vj;
				vj = temp;
				setLinearElementAt(head,i, vi);
				setLinearElementAt(head,j, vj);
			}
		}
	}
}

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
	puts("\n");
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
}


int main(void){
	LINEAR *polyPoint1 = NULL;
	LINEAR *polyPoint2 = NULL;
	LINEAR *res = NULL;

	initLinear(&polyPoint1, POLYNOMIAL_ELEMENT_MAX_COUNT);
	initLinear(&polyPoint2, POLYNOMIAL_ELEMENT_MAX_COUNT);

	printf("请输入第一个多项式：\n");
	initPolynomial(polyPoint1);
	sortPolynomialByPower(*polyPoint1);
	
	printf("请输入第二个多项式：\n");
	initPolynomial(polyPoint2);
	sortPolynomialByPower(*polyPoint2);
	
	printf("\n第一个多项式：\n");
	showPolynomial(*polyPoint1);	
	printf("\n第二个多项式：\n");
	showPolynomial(*polyPoint2);	
	
	res = polynomialAdd(*polyPoint1, *polyPoint2);
	printf("\n两个多项式求和的结果是：\n");
	showPolynomial(*res);	

	destoryLinear(&polyPoint1);
	destoryLinear(&polyPoint2);
	
	return 0;
}

