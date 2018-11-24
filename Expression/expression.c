//fileName:	expression.c
//author:	jonewan
//date:		2018/11/22

#include <stdio.h>
#include <ctype.h>

#define	BEGIN			2
#define	INTEGER		3
#define	DOT				4
#define DECIMIAL	5
#define	SIGN			6
#define	END				7		

typedef unsigned char	boolean;

#define TRUE	1
#define	FALSE	0

boolean getNum(char *str, double *result, int *index);

boolean getNum(char *str, double *result, int *index){
	int status = BEGIN;
	boolean ok = TRUE;
	boolean finish = FALSE;
	int i = *index;
	double res = 0.0;
	double decimal = 0.1;
	int sign = 1;
	int decimalCount = 1;

	if(str == NULL)
		return FALSE;

	while(ok && !finish){
		if(status == BEGIN){
			puts("status is BEGIN\n");
			if(isdigit(str[i])){
				res = res * 10 + str[i] - '0';
				status = INTEGER;
				i++;
			}
			else if(str[i] == '.'){
				puts("to DOT!\n");
				status = DOT;
				i++;
			}
			else if(str[i] == '+' || str[i] == '-'){
				if(str[i] == '-')
					sign = -1;
				status = SIGN;
				i++;
			}
			else{
				//这里处理错误：出师未捷身先死！
				printf("\n出师未捷身先死！\n");
				ok = FALSE;
			}
		}
		else if(status == INTEGER){
			puts("status is INTEGER\n");
			if(isdigit(str[i])){
				res = res * 10 + str[i] - '0';
				i++;
			}
			else if('.' == str[i]){
				status = DOT;
				i++;
			}
			else{
				status = END;
			}
		}
		else if(DOT == status){
			puts("status is DOT\n");
			if(isdigit(str[i])){
				res += decimal * (str[i] - '0');
				decimal /= 10;
				i++;
				status = DECIMIAL;
			}
			else{
				//这里处理错误：出师未捷身先死！
				puts("\n缺乏小数位！\n");
				ok = FALSE;
			}
		}
		else if(status == DECIMIAL){
			puts("status is DECIMIAL\n");
			if(isdigit(str[i])){
				if(++decimalCount < 7){
					res += decimal * (str[i] - '0');
					decimal /= 10;
					i++;
				}
				else{
				//这里处理错误：出师未捷身先死！
					puts("\n小数位过多（不大于6位小数）！\n");
					ok = FALSE;
				}
			}
			else{
				status = END;
			}
		}
		else if(status == SIGN){
			puts("status is SIGN\n");
			if(isdigit(str[i])){
				res = res * 10 + str[i] - '0';
				status = INTEGER;
				i++;
			}
			else if(str[i] == '.'){
				status = DOT;
				i++;
			}
			else{
				//这里处理错误：出师未捷身先死！
				printf("无效的数值！\n");
				ok = FALSE;				
			}
		}
		else if(status == END){
			puts("status is END\n");
			finish = TRUE;
			*result = res * sign;
			*index = i;
		}
	}

	return ok;
}

int main(void){
	char str[80] = {0};
	double result = 0.0;
	boolean ok;
	int index = 0;

	printf("请输入一个数值：");
	gets(str);

	ok = getNum(str, &result, &index);
	if(ok)
		printf("%lf\n", result);
	else
		printf("数值非法！\n");

	return	0;
}