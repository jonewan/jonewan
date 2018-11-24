//fileName:     queue.h
//author:       jonewan
//date:         2018/11/24

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <malloc.h>

typedef unsigned int    boolean;

#define TRUE    1
#define FALSE   0

typedef struct QUEUE{
	USET_TYPE	*data;			//它将指向队列空间
	int queueMaxRoom;			//队列最大空间
	int queueHead;					//队首指针（下标）
	int queueTail;					//队尾指针（下标）
	int lastAction;			//末次操作类型（IN/OUT）
} QUEUE;

#define IN          1
#define OUT     0

boolean initQueue(QUEUE **head, int maxRoom);
boolean destoryQueue(QUEUE **head);
boolean isQueueEmpty(QUEUE head);
boolean isQueueFull(QUEUE head);
boolean in(QUEUE *head, USER_TYPE value);
boolean out(QUEUE *head, USER_TYPE *value);
boolean readHead(QUEUE head, USER_TYPE *value);

boolean readHead(QUEUE head, USER_TYPE *value){
    boolean ok = TRUE;

    if(!isQueueEmpty(head))
        *value = head.data[head.queueHead];
    else
        ok = FALSE;

    return ok;
}

boolean out(QUEUE *head, USER_TYPE *value){
    boolean ok = TRUE;

    if(head != NULL && !isQueueEmpty(*head)){
        *value = head->data[head->queueHead];
        head->queueHead = (head->queueHead + 1) % head->queueMaxRoom;
        head->lastAction = OUT;
    }
    else
        ok = FALSE;

    return ok;
}

boolean in(QUEUE *head, USER_TYPE value){
    boolean ok = TRUE;

    if(head != NULL && !isQueueFull(*head)){
        head->data[head->queueTail] = value;
        head->queueTail = (head->queueTail + 1) % head->queueMaxRoom;
        head->lastAction = IN;
    }
    else
        ok = FALSE;

    return ok;
}

boolean isQueueFull(QUEUE head){
    return lastAction == IN && head.queueHead == head.queueTail;
}

boolean isQueueEmpty(QUEUE head){
    return lastAction == OUT && head.queueHead == head.queueTail;
}

boolean destoryQueue(QUEUE **head){
    boolean ok = TRUE;

    if(head == NULL || *head == NULL)
        return FALSE;

    if((*head)->data != NULL){
        free((*head)->data);
    }
    free(*head);
    *head = NULL;

    return ok;
}

boolean initQueue(QUEUE **head, int maxRoom){
    boolean ok = TRUE;

    if(head == NULL)
        return FALSE;

    if(*head != NULL || maxRoom <= 0){
        ok = FALSE;
    }
    else{
        *head = (QUEUE*)malloc(sizeof(QUEUE);
        if(*head == NULL){
            ok = FALSE;
        }
        else{
            (*head)->queueMaxRoom = maxRoom;
            (*head)->data = (USER_TYPE *)malloc(sizeof(USER_TYPE) * maxRoom);
            if((*head)->data != NULL){
                (*head)->queueHead = (*head)->queueTail = 0;
                (*head)->lastAction = OUT;
            }else{
                ok = FALSE;
                free(*head);
                *head = NULL;
            }
        }
    }

    return ok;
}

#endif
