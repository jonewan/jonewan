//fileName:	expression.c
//author:	jonewan
//date:		2018/11/24
//discription:	处理带括号的表达式

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
int getOperatorIndex(char c);
boolean isOPerator(char c);
int skipBlank(char *str, int i);
int cmpCurOperWithStackTopOper(char oper, STACK operStack);
boolean calculate(STACK *numStack, STACK *operStack,  char oper);
boolean compute(STACK *numStack, STACK *operStack);
boolean lastCompute(STACK *numStack, STACK *operStack);
boolean Account(double leftOperand, double rightOperand, char oper, double *result);

boolean isOperStackEmpty(STACK stack, char emptyFlag);
boolean computeUntilLeftBracket(STACK *numStack, STACK *operStack);

boolean computeUntilLeftBracket(STACK *numStack, STACK *operStack){
    boolean ok = TRUE;

    while(ok && !isOperStackEmpty(*operStack, '('))
        ok = compute(numStack, operStack);

    return ok;
}

boolean isOperStackEmpty(STACK stack, char emptyFlag){
	ELEMENT	v;
	boolean ok = TRUE;

    if(isStackEmpty(stack) == FALSE){
        readTop(stack, &v);
        if(v.oper != emptyFlag)
            ok = FALSE;
    }

	return ok;
}

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

    while(ok && !isOperStackEmpty(*operStack, '('))
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

    pop(operStack, &v);//取运算符
    oper = v.oper;
/*************************/
    printf("\n计算 %lf %c %lf\n", leftOperand, oper, rightOperand);
/****************************/
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
    while(noError && !isOperStackEmpty(*operStack, '(') && cmpCurOperWithStackTopOper(oper, *operStack) == LOW)
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


boolean isOPerator(char c){
    return getOperatorIndex(c) != NOT_OPERATOR;
}

int getOperatorIndex(char c){
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
				printf("\n出师未捷身先死！\n");
				ok = FALSE;
			}
		}
		else if(status == INTEGER){
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
			if(isdigit(str[i])){
				res += decimal * (str[i] - '0');
				decimal /= 10;
				i++;
				status = DECIMIAL;
			}
			else{
				puts("\n缺乏小数位！\n");
				ok = FALSE;
			}
		}
		else if(status == DECIMIAL){
			if(isdigit(str[i])){
				if(++decimalCount < 7){
					res += decimal * (str[i] - '0');
					decimal /= 10;
					i++;
				}
				else{
					puts("\n小数位过多（不大于6位小数）！\n");
					ok = FALSE;
				}
			}
			else{
				status = END;
			}
		}
		else if(status == SIGN){
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
				printf("无效的数值！\n");
				ok = FALSE;				
			}
		}
		else if(status == END){
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
    int stackMaxRoom = strlen(str);
    char temp;
    USER_TYPE   numValue;
    USER_TYPE   operValue;
    int bracketMatch = 0;

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
                    //printf("\n%lf",v);
                    numValue.num = v;
                    push(numStack, numValue);
                    status = AFTER_NUM;
                }
                else{
                    printf("\n无效的运算数！\n");
                    ok = FALSE;
                }
            }
            else if(str[i] == '('){//处理左括号
                    bracketMatch++;
                    operValue.oper = '(';
                    push(operStack, operValue);
                    i++;
            }
            else{
                printf("缺少运算数！\n");
                    ok = FALSE;
            }
        }
        else if(status == AFTER_NUM){
            i = skipBlank(str, i);
            if(isOPerator(str[i])){
                //1、进行运算
                ok = calculate(numStack, operStack, str[i]);
                if(ok){
                    status = BEGIN;
                    i++;
                }
            }
            else if(str[i] == ')'){
                if(--bracketMatch < 0){
                    printf(")多余\n");
                    ok = FALSE;
                }
                else{
                    //处理右括号
                    ok = computeUntilLeftBracket(numStack, operStack);
                    if(ok)
                        pop(operStack, &numValue);
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
            if(bracketMatch != 0){
                ok = FALSE;
                printf("(多余\n");
            }
            else{
                if(ok){
                    pop(numStack, &numValue);
                    *result = numValue.num;
                    finished = TRUE;
                }
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