#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "jonewan.h"

void getNext(const char *str, int *next);
int kmpSearch(const char *str, const char *subStr);

int kmpSearch(const char *str, const char *subStr){
	int strLen = strlen(str);
	int subLen = strlen(subStr);
	int *next = NULL;
	int i, j;

	if(strLen <= 0 || subLen <= 0 || strLen < subLen)
		return -1;
	next = (int *)calloc(sizeof(int), subLen);
	if(subLen > 2){
		getNext(subStr, next);
	}

	i = j = 0;
	while(strLen -i + j >= subLen){
		while(subStr[j] && str[i] == subStr[j]){
			i++; j++;
		}
		if(subStr[j] == 0){
			free(next);
			return i - subLen;
		}
		else if(j == 0){
			i++;
			j = 0;
		}
		else{
			j = next[j];
		}
	}

	free(next);

	return -1;
}

void getNext(const char *str, int *next){
	int i = 0;
	int j = 0;
	boolean flag = TRUE;

	next[0] = next[1] = 0;
	for(i = 2; str[i]; i++){
		for(flag = TRUE; flag;){
			if(str[i-1] == str[j]){
				next[i] = ++j;
				flag = FALSE;
			}
			else if(j == 0){
				next[i] = 0;
				flag = FALSE;
			}
			else{
				j = next[j];
			}
		}
	}

}

int main(void){
	char str[80];
	char subStr[80];
	int result;

	printf("请输入源字符串:");
	gets(str);
	printf("\n请输入子字符串:");
	gets(subStr);

	result = kmpSearch(str, subStr);
	if(result == -1){
		printf("字符串[%s]不存在子串[%s]\n",str, subStr);
	}
	else{
		printf("子串[%s]第一次出现在字符串[%s]中的下标为%d\n", subStr, str, result);
	}
	
	
	
	return 0;
}
