//fileName:	adaptBracket.c
//author:	jonewan
//date:		2018/11/22
//description:	括号匹配问题，输入的字符串中只有[]{}两种括号，要求给出时间复杂度为O（N）的解决方案：
//设定两个栈顶指针：middleMatch和bigMatch，并设其初值为0，对字符串进行遍历，遇到左括号相应的栈顶指针++，遇到与之匹配的右括号，相应的栈顶指针--，若其中任意一个栈顶指针在遍历字符串的过程中为负数，则代表右括号多了，不匹配，若遍历完成字符串后相应的栈顶指针不为零，则代表括号不匹配，并且可以得知不匹配的原因：
//若栈顶指针大于零，则代表左括号多了，若小于零则代表右括号多了

#include <stdio.h>
#include <string.h>

typedef unsigned int boolean;

#define TRUE	1
#define FALSE	0

boolean isBracketMatch(char *str);

boolean isBracketMatch(char *str){
	boolean ok = TRUE;
	int middleMatch = 0;
	int bigMatch = 0;
	int index = 0;

	while(index < strlen(str) && bigMatch >=0 && middleMatch >= 0){
		if(str[index] == '[')
			middleMatch++;
		else if(str[index] == '{')
			bigMatch++;
		else if(str[index] == ']')
			middleMatch--;
		else if(str[index]== '}')
			bigMatch--;

		index++;
	}
	if(bigMatch != 0 || middleMatch != 0)
		ok = FALSE;

	return ok;
}

int main(){
	char str[80];
	puts("please input bracket string:");
	gets(str);
	if(TRUE == isBracketMatch(str))
		puts("\nbracket is match!!!\n");
	else
		puts("\nbracket not match!!!\n");
	
	return 0;
}
