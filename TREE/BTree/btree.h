//fileName: btree.h
//author:   jonewan
//date:     2018/11/27

#ifndef _BTREE_H_
#define _BTREE_H_

#include <stdio.h>
#include <malloc.h>
#include <ctype.h>
#include <string.h>

typedef struct BTREE{
    DATA_TYPE   data;
    struct BTREE *leftChild;
    struct BTREE *rightChild;
}BTREE;

typedef BTREE   *USER_TYPE;

#include "stack.h"

typedef unsigned char   CHILD_TYPE;

#define LEFT_CHILD      0
#define RIGHT_CHILD     1

#define BEGIN						1
#define ALPHA           2
#define LEFT_BRACKET    3
#define RIGHT_BRACKET   4
#define COMMA           5
#define END             6

#define NO_ERROR																-1
#define ERROR_BEGIN_CODE                        0
#define ERROR_NOT_SINGLE_ALPHA                  1
#define ERROR_MISSING_LEFT_BRACKET              2
#define ERROR_MISSING_RIGHT_CHILD               3
#define ERROR_MISSING_LEFT_CHILD                4
#define ERROR_ILLEGAL_CODE_AFTER_RIGHT_BRACKET  5
#define ERROR_MISSING_RIGHT_BRACKET             6
#define ERROR_TOO_MANY_ROOT                     7
#define ERROR_TOO_MANY_CHILDREN                 8

int errNo = NO_ERROR;
const char *errMsg[] = {
    "二叉树规范式应该以根结点数据（必须是单个大写字母）开始！",
    "二叉树中所存储的数据只能是单个的大写字母！",
    "缺少左括号！",
    "逗号后缺少右孩子数据；若无右孩子，则，不允许有逗号！",
    "左括号后缺少左孩子数据！","右括号后出现了非法字符！",
    "缺少右括号！",
    "根结点太多！",
    "孩子太多了！",
};

boolean initBtree(BTREE **rootPoint, char *);
int skipBlank(char *str, int i);
boolean processNode(char data, BTREE **pP, BTREE **pRoot, STACK stack, int childFlag);
void destoryBtree(BTREE **root);
void preRootvisitBtree(BTREE *root);
void midRootvisitBtree(BTREE *root);
void lastRootvisitBtree(BTREE *root);

void lastRootvisitBtree(BTREE *root){
    if(root != NULL){
        lastRootvisitBtree(root->leftChild);
        lastRootvisitBtree(root->rightChild);
        printf("%c ", root->data);
    }
}

void midRootvisitBtree(BTREE *root){
    if(root != NULL){
        midRootvisitBtree(root->leftChild);
        printf("%c ", root->data);
        midRootvisitBtree(root->rightChild);
    }
}

void preRootvisitBtree(BTREE *root){
    if(root != NULL){
        printf("%c ", root->data);
        preRootvisitBtree(root->leftChild);
        preRootvisitBtree(root->rightChild);
    }
}

void destoryBtree(BTREE **root){
    if(root == NULL)
        return ;
    if(*root != NULL){
        destoryBtree(&((*root)->leftChild));
        destoryBtree(&((*root)->rightChild));
        free(*root);
    }
}

boolean processNode(char data, BTREE **pP, BTREE **pRoot, STACK stack, int childFlag){
    boolean ok = TRUE;
    USER_TYPE stackTop;
    BTREE *p = *pP;
    BTREE *root = *pRoot;

    p = (BTREE *)malloc(sizeof(BTREE));
    p->data = data;
    p->leftChild = p->rightChild = NULL;
    if(root == NULL)
        root = p;
    else if(isStackEmpty(stack) == TRUE){
        ok = FALSE;
        errNo = ERROR_TOO_MANY_ROOT;
        free(p);
    }
    else{
        readTop(stack, &stackTop);
        if(childFlag == LEFT_CHILD){
            stackTop->leftChild = p;
        }
        else if(childFlag == RIGHT_CHILD){
            if(stackTop->rightChild == NULL)
                stackTop->rightChild = p;
            else{
                ok = FALSE;
                errNo = ERROR_TOO_MANY_CHILDREN;
                free(p);
            }
        }
    }

    if(ok){
        *pP = p;
        *pRoot = root;
    }

    return ok;
}

int skipBlank(char *str, int i){
    while(str[i] && isspace(str[i]) )
        i++;
        return i;
}

boolean initBtree(BTREE **rootPoint, char *content){
    boolean ok = TRUE;
    boolean finished = FALSE;
    int status = BEGIN;
    int i = 0;
    int bracketMatch = 0;
    BTREE *root = NULL;
    BTREE *p = NULL;
    STACK *stack = NULL;
    CHILD_TYPE childFlag = LEFT_CHILD;

    if(initStack(&stack, strlen(content)) == FALSE)
        return FALSE;

    if(rootPoint == NULL)
        return FALSE;

    if(*rootPoint != NULL)
        ok = FALSE;
    else{
        while(ok && !finished){
            i = skipBlank(content, i);
            switch(status){
                case BEGIN:
                    if(isupper(content[i])){
                        //处理这个节点
                        ok = processNode(content[i], &p, &root, *stack, childFlag);
                        if(ok){
                            status = ALPHA;
                            i++;
                        }
                    }
                    else if(content[i] == '\0')
                        status = END;
                    else{
                        errNo = ERROR_BEGIN_CODE;
                        ok = FALSE;
                    }
                    break;
                case ALPHA:
                    switch(content[i]){
                        case '(':
                            //处理左括号
                            push(stack, p);
                            childFlag = LEFT_CHILD;
                            bracketMatch++;
                            status = LEFT_BRACKET;
                            i++;
                            break;
                        case ')':
                            //处理右括号
                            if(--bracketMatch < 0){
                                ok = FALSE;
                                errNo = ERROR_MISSING_LEFT_BRACKET;
                            }
                            else{
                                USER_TYPE   temp;
                                pop(stack, &temp);
                                status = RIGHT_BRACKET;
                                i++;
                            }
                            break;
                        case ',':
                            //处理逗号
                            childFlag = RIGHT_CHILD;
                            status = COMMA;
                            i++;
                        case '\0':
                            status = END;
                            break;
                        default:
                            ok = FALSE;
                            errNo = ERROR_NOT_SINGLE_ALPHA;
                            printf("\nnow is %c\n",content[i]);
                    }
                    break;
                case LEFT_BRACKET:
                    if(isupper(content[i])){
                        //处理这个节点
                        ok = processNode(content[i], &p, &root, *stack, childFlag);
                        if(ok){
                            status = ALPHA;
                            i++;
                        }
                    }
                    else if(content[i] == ','){
                        //处理逗号
                        childFlag = RIGHT_CHILD;
                        status = COMMA;
                        i++;
                    }
                    else{
                        ok = FALSE;
                        errNo = ERROR_MISSING_LEFT_CHILD;
                    }
                    break;
                case RIGHT_BRACKET:
                    switch(content[i]){
                        case ',':
                            if(content[i] == ','){
                                //处理逗号
                                childFlag = RIGHT_CHILD;
                                status = COMMA;
                                i++;
                            }
                            break;
                        case ')':
                            //处理右括号
                            if(--bracketMatch < 0){
                                ok = FALSE;
                                errNo = ERROR_MISSING_LEFT_BRACKET;
                            }
                            else{
                                USER_TYPE   temp;
                                pop(stack, &temp);
                                status = RIGHT_BRACKET;
                                i++;
                            }
                            break;
                        case '\0':
                            status = END;
                            break;
                        default:
                            ok = FALSE;
                            errNo = ERROR_ILLEGAL_CODE_AFTER_RIGHT_BRACKET;
                    }
                    break;
                case COMMA:
                    if(isupper(content[i])){
                        //处理这个节点
                        ok = processNode(content[i], &p, &root, *stack, childFlag);
                        if(ok){
                            status = ALPHA;
                            i++;
                        }
                    }
                    else{
                        ok = FALSE;
                        errNo = ERROR_MISSING_RIGHT_CHILD;
                    }
                    break;
                case END:
                    if(bracketMatch == 0){
                        (*rootPoint) = root;
                        finished = TRUE;
                    }
                    else{
                        ok = FALSE;
                        errNo = ERROR_MISSING_RIGHT_BRACKET;
                    }
                    break;
            }
        }
    }

    if(ok = FALSE){
        destoryBtree(&root);
        *rootPoint = NULL;
    }

    destoryStack(&stack);

    return ok;
}

#endif