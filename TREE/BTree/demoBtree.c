//fileName: demoBtree.c
//author:   jonewan
//date:     2018/11/27

#include <stdio.h>

typedef char DATA_TYPE;

#include "btree.h"

int main(void){
    BTREE   *btp = NULL;
    char str[80] = {0};
    boolean ok;

    printf("请输入一个二叉树规范式： ");
    
    gets(str);
    
    ok = initBtree(&btp, str);

    if(FALSE == ok && NO_ERROR != errNo)
        printf("%s\n", errMsg[errNo]);
    else
        printf("\n表达式无错误!\n");

    if(btp != NULL){
        puts("先根序访问结果:\n");
        preRootvisitBtree(btp);
        puts("\n中根序访问结果:\n");
        midRootvisitBtree(btp);
        puts("\n后根序访问结果:\n");
        lastRootvisitBtree(btp);
    }

    destoryBtree(&btp);

    return 0;
}
