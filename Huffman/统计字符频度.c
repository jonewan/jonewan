//fileName:		统计字符频度.c
//author:			jonewan
//date:				2018/11/27

#include <stdio.h>
#include <string.h>

int main(void){
    int freq[256] = {0};
    int i = 0;
    char str[80];

    puts("请输入一个字符串:\n");
    gets(str);
    for(i = 0; str[i]; i++)
        freq[str[i]]++;
    for(i = 0; i < 256; i++)
        if(freq[i])
            printf("字符：%c\t频度：%d\n", i, freq[i]);
    
    return 0;
}
