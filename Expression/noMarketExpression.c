//fileName:     noMarketExpression.c
//author:       jonewan
//date:         2018/11/24
/*
    description:  不带括号的表达式运算
    在gcc下编译程序时，如果i使用了nclude <math.h>的函数，则需要在编译命令后加上 -lm(代表link math libiary)
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

typedef union ELEMENT{
    double  num;
    char    oper;
}ELEMENT;

typedef ELEMENT USER_TYPE;

#include "stack.h"

#define BEGIN           0
#define END             1
#define	INTEGER         2
#define	DOT             3
#define DECIMIAL        4
#define	SIGN            5
#define AFTER_NUM   6

#define OPERATOR_MAX_COUNT  5
#define HIGH     1
#define LOW     0
#define NOT_OPERATOR    -1

const char *operStr = "+-*/^";
//operMatrix[当前运算符下标][栈顶运算符下标]
const int operMatrix[][OPERATOR_MAX_COUNT] = {
//                +           -          *           /           ^       <-----栈顶运算符
/* + */     LOW,    LOW,     LOW,    LOW,    LOW,
/* - */     LOW,    LOW,     LOW,    LOW,    LOW,
/* * */     HIGH,    HIGH,     LOW,    LOW,    LOW,
/* / */     HIGH,    HIGH,     LOW,    LOW,    LOW,
/* ^ */     HIGH,    HIGH,     HIGH,    HIGH,    HIGH
//当前运算符
};

boolean processExpression(char *s, double *result);
boolean isOperand(char c);//判断是操作数
boolean getNum(char *str, double *result, int *index);//取出数值
int isOPerator(char c);
int skipBlank(char *str, int i);
int cmpCurOperWithStackTopOper(char oper, STACK operStack);
boolean calculate(STACK *numStack, STACK *operStack,  char oper);
boolean compute(STACK *numStack, STACK *operStack);
boolean lastCompute(STACK *numStack, STACK *operStack);
boolean Account(double leftOperand, double rightOperand, char oper, double *result);

boolean Account(double leftOperand, double rightOperand, char oper, double *result){
    boolean ok = TRUE;

    switch(oper){
        case '+'   :
            *result = leftOperand+rightOperand;
            break;
        case '-'   :
            *result = leftOperand-rightOperand;
            break;
        case '*'   :
            *result = leftOperand*rightOperand;
            break;
        case '/'   :
            if(fabs(rightOperand) <= 1e-6){
                puts("\n除零错！\n");
                ok = FALSE;
            }
            else
                *result = leftOperand/rightOperand;
            break;
        case '^'   :
                *result = pow(leftOperand,rightOperand);
            break;
    }

    return ok;
}

boolean lastCompute(STACK *numStack, STACK *operStack){
    boolean ok = TRUE;

    while(ok && !isStackEmpty(*operStack))
        ok = compute(numStack, operStack);

    return ok;
}

boolean compute(STACK *numStack, STACK *operStack){
    USER_TYPE   v;
    boolean ok = TRUE;
    double leftOperand;
    double rightOperand;
    char oper;
    double result;

    pop(numStack, &v);//取右运算数
    rightOperand = v.num;
    
    pop(numStack, &v);//取左运算数
    leftOperand = v.num;

    pop(operStack, &v);//取右运算数
    oper = v.oper;

    ok = Account(leftOperand, rightOperand, oper, &result);
    if(ok){
        v.num = result;
        push(numStack, v);
    }

    return ok;
}

boolean calculate(STACK *numStack, STACK *operStack,  char oper){
    boolean noError = TRUE;
    USER_TYPE   v;

    v.oper = oper;
    while(noError && !isStackEmpty(*operStack) && cmpCurOperWithStackTopOper(oper, *operStack) == LOW)
        compute(numStack, operStack);
    if(noError)
        push(operStack, v);

    return noError;
}

int cmpCurOperWithStackTopOper(char oper, STACK operStack){
    char stackTopOper;
    USER_TYPE   operValue;

    readTop(operStack, &operValue);
    stackTopOper = operValue.oper;

    return operMatrix[isOPerator(oper)][isOPerator(stackTopOper)];
}

int skipBlank(char *str, int i){
    while(str[i] && isspace(str[i]) )
        i++;
        return i;
}


int isOPerator(char c){
    int index;

    for(index = 0; operStr[index] && operStr[index] != c; index++)
        ;
    if(operStr[index] == 0)
        index = NOT_OPERATOR;

    return index;
}

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
			//puts("status is BEGIN\n");
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
			//puts("status is INTEGER\n");
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
			//puts("status is DOT\n");
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
			//puts("status is DECIMIAL\n");
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
			//puts("status is SIGN\n");
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
			//puts("status is END\n");
			finish = TRUE;
			*result = res * sign;
			*index = i;
		}
	}

	return ok;
}

boolean isOperand(char c){
    return isdigit(c) || c == '.' || c == '+' || c == '-' ;
}

boolean processExpression(char *str, double *result){
    STACK   *numStack = NULL;
    STACK   *operStack = NULL;
    boolean ok = TRUE;
    boolean finished = FALSE;
    int status = BEGIN;
    double res = 0.0;
    double v = 0.0;
    int i = 0;
    int stackMaxRoom = (strlen(str)+1)/2;
    char temp;
    USER_TYPE   numValue;
    USER_TYPE   operValue;

    initStack(&numStack, stackMaxRoom);
    initStack(&operStack, stackMaxRoom);

    while(!finished && ok){
        if(status == BEGIN){
            i = skipBlank(str, i);
            if(isOperand(str[i]) == TRUE){
                //1、取出这个数=>v
                ok = getNum(str, &v, &i);
                if(ok){
                    //2、处理运算数v
                    numValue.num = v;
                    push(numStack, numValue);
                    status = AFTER_NUM;
                    printf("\n%lf",v);
                }
                else{
                    printf("\n无效的运算数！\n");
                    ok = FALSE;
                }
            }
            else{
                printf("缺少运算数！\n");
                    ok = FALSE;
            }
        }
        else if(status == AFTER_NUM){
            i = skipBlank(str, i);
            if(isOPerator(str[i]) != NOT_OPERATOR){
                //1、进行运算
                ok = calculate(numStack, operStack, str[i]);
                if(ok){
                    status = BEGIN;
                    printf("\n%c", str[i]);
                    i++;
                }
            }
            else if(str[i] == 0){
                status = END;
            }
            else{
                temp = str[i];
                str[i] = 0;
                printf("\"%s\"后表达式中存在无效字符！\n", str, temp);
                str[i] = temp;
                ok = FALSE; 
            }
        }
        else if(status = END){
            //将运算结果传回给主调函数
            ok = lastCompute(numStack, operStack);
            if(ok){
                pop(numStack, &numValue);
                *result = numValue.num;
                finished = TRUE;
            }
        }
    }
    destoryStack(&numStack);
    destoryStack(&operStack);

    return ok;
}

int main(void){
    char s[128];
    boolean ok;
    double result = 0;

    printf("请输入一个表达式： ");
    gets(s);

    ok = processExpression(s, &result);

    if(ok)
        printf("\n%s的计算结果是%lf\n", s, result);
    else   
        printf("\n错误!\n");

    return 0;
}