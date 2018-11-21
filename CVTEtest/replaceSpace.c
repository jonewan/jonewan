//fileName:		replaceSpace.c
//author:		jonewan
//date:			2018/11/21
/*description:请实现一个函数，把字符串中每个空格替换成"%20"。实现时间复杂度为O(n)！！！
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

void exchangeStr(char *str);
int enlargeStrLen(char *str);

int enlargeStrLen(char *str){
	int i = 0;
	int spaceCount = 0;

	for(i = 0; i < strlen(str); i++)
		if(isspace(str[i]))
			spaceCount++;
	return strlen(str)+spaceCount*2;
}

void exchangeStr(char *str){
	int newTailIndex;
	int oldTailIndex = strlen(str);

	newTailIndex = enlargeStrLen(str);
	printf("newStrLen = %d\n", newTailIndex);
	while(newTailIndex >= 0 && oldTailIndex >= 0 && newTailIndex > oldTailIndex){
		if(!isspace(str[oldTailIndex])){
			str[newTailIndex] = str[oldTailIndex];
			printf("oldTailIndex = %d, newTailIndex = %d\n", oldTailIndex, newTailIndex);
		}
		else{
			str[newTailIndex--] = '0';
			str[newTailIndex--] = '2';
			str[newTailIndex] = '%';
			printf("oldTailIndex = %d, newTailIndex = %d\n", oldTailIndex, newTailIndex);
		}
		newTailIndex--;
		oldTailIndex--;
	}
	
}

int main(void){
	char str[80] = {0};

	puts("please input str with space: ");
	gets(str);
	puts("your input str is: ");
	puts(str);	
	puts("\n");
	printf("strLen = %d\n", strlen(str));
	exchangeStr(str);
	puts(str);	
	puts("\n");
	printf("strLen = %d\n", strlen(str));
		

	return 0;
}
