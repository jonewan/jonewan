//fileName:	stack.h
//author:	jonewan
//date:		2018/11/22

#ifndef _STACK_H_
#define _STACK_H_

#include <stdio.h>
#include <malloc.h>

typedef struct STACK{
	USER_TYPE	*data;
	int	stackMaxRoom;
	int top;
}STACK;

typedef unsigned int boolean;

#define TRUE	1
#define	FALSE	0

boolean initStack(STACK **head, int maxRoom);
boolean destoryStack(STACK **head);
boolean isStackEmpty(STACK head);
boolean isStackFull(STACK head);
boolean push(STACK *head, USER_TYPE value);
boolean pop(STACK *head, USER_TYPE *value);
boolean readTop(STACK head, USER_TYPE *value);

boolean isStackEmpty(STACK head){
	return head.top <= 0;
}

boolean isStackFull(STACK head){
	return head.top >= head.stackMaxRoom;
}

boolean push(STACK *head, USER_TYPE value){
	boolean ok = TRUE;
	
	if(head == NULL)
		return FALSE;

	if(isStackFull(*head) == TRUE)
		ok = FALSE;
	else
		head->data[head->top++] = value;

	return ok;
}

boolean pop(STACK *head, USER_TYPE *value){
	boolean ok = TRUE;

	if(head == NULL)
		return FALSE;

	if(isStackEmpty(*head) == TRUE)
		ok = FALSE;
	else
		*value = head->data[--head->top];
	
	return ok;
}

boolean readTop(STACK head, USER_TYPE *value){
	boolean ok = TRUE;

	if(isStackEmpty(head) == TRUE)
		ok = FALSE;
	else
		*value = head.data[head.top-1];

	return ok;
}


boolean destoryStack(STACK **head){
	boolean ok = TRUE;

	if(head == NULL)
		return FALSE;

	if(*head == NULL)
		ok = FALSE;
	else{
		if((*head)->data != NULL)
			free((*head)->data);
		free((*head));
		*head = NULL;
	}

	return ok;
}

boolean initStack(STACK **head, int maxRoom){
	boolean ok = TRUE;

	if(head == NULL)
		return FALSE;

	if(maxRoom <= 0)
		ok = FALSE;
	else{
		if(*head == NULL){
			*head = (STACK *)malloc(sizeof(STACK));
			if(NULL == (*head)){
				ok = FALSE;
			}
			else{
				(*head)->data = (USER_TYPE*)malloc(sizeof(USER_TYPE)*maxRoom);
				if((*head)->data != NULL){
					(*head)->stackMaxRoom = maxRoom;
					(*head)->top = 0;
				}
				else{
					free(*head);
					*head = NULL;
					ok = FALSE;
				}
			}
		}
		else
			ok = FALSE;
	}

	return ok;
}

#endif

