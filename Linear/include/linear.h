//fileName:	linear.h
//author:	jonewan
//date:		2018/11/19

#ifndef _LINEAR_H_
#define _LINEAR_H_

typedef struct LINEAR{	//线性表的控制头
	USER_TYPE	*data;	//data用以指向线性存储空间
	int linearMaxRoom;	//线性表最大空间
	int count;			//线性表的有效元素个数
} LINEAR;

void initLinear(LINEAR **head,int maxRoom);
void destoryLinear(LINEAR **head);

void destoryLinear(LINEAR **head){
	if(*head == NULL)	return;

	
}

void initLinear(LINEAR **head,int maxRoom){
	if(*head != NULL)	return;

	(*head) = (LINEAR *)malloc(sizeof(LINEAR));
	(*head)->data = (USER_TYPE *)malloc(sizeof(USER_TYPE));
	(*head)->linearMaxRoom = maxRoom;
	(*head)->count = 0;
}









#endif

