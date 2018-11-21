//fileName:	linear.h
//author:	jonewan
//date:		2018/11/19

#ifndef _LINEAR_H_
#define _LINEAR_H_

#include <malloc.h>

/*
 *	在线性表工具中，有如下运算：
 *	1、判空；
 *	2、判满；
 *	3、追加，在末节点的后面增加新节点；
 *	4、插入，将新节点插入到指定下标的当前位置；
 *	5、按指定下标删除节点；
 *	6、按指定数据删除节点；
 *	7、查找指定数据节点的下标；
 *	8、设置指定下标位置节点的数据；
 *	9、取得指定下标位置节点的数据；
 *	......
 */


typedef struct LINEAR{	//线性表的控制头
	USER_TYPE	*data;	//data用以指向线性存储空间
	int linearMaxRoom;	//线性表最大空间
	int count;			//线性表的有效元素个数
} LINEAR;

#define TRUE			1
#define	FALSE			0
#define	NOT_FOUND		-1

typedef unsigned int	boolean;

void initLinear(LINEAR **head,int maxRoom);
void destoryLinear(LINEAR **head);
boolean isLinearEmpty(LINEAR head);
boolean isLinearFull(LINEAR head);
boolean getLinearElementAt(USER_TYPE *data, LINEAR head, int index);
int getLinearIndex(USER_TYPE data, LINEAR head, int (*cmp)());
boolean setLinearElementAt(LINEAR head, int index, USER_TYPE value);
boolean appendLinear(LINEAR *head, USER_TYPE value);
boolean insertLinearAt(LINEAR *head, int index, USER_TYPE value);
boolean isIndexValid(LINEAR head, int index);
int getLinearElementCount(LINEAR head);
int getLinearMaxRoom(LINEAR head);
boolean removeLinearElementAt(LINEAR *head, int index);
boolean removeLinearEnement(LINEAR *head, USER_TYPE value, int (*cmp)());

boolean removeLinearEnement(LINEAR *head, USER_TYPE value, int (*cmp)()){
	boolean ok = FALSE;
	int index = getLinearIndex(value, *head, cmp);

	if(index != NOT_FOUND)
		ok = removeLinearElementAt(head, index);
	return ok;
}

boolean removeLinearElementAt(LINEAR *head, int index){
	boolean ok = TRUE;
	int i = 0;

	if(TRUE == isLinearEmpty(*head) || FALSE == isIndexValid(*head, index))
		ok = FALSE;
	else{
	//1、从index+1开始以后的所有元素，向前移动一个元素
		for(i = index; i < head->count-1; i++)
			head->data[i] = head->data[i+1];
	//2、count减1
		head->count--;
	}

	return ok;
}

int getLinearMaxRoom(LINEAR head){
	return head.linearMaxRoom;
}

int getLinearElementCount(LINEAR head){
	return head.count;
}

boolean isIndexValid(LINEAR head, int index){
	return index >= 0 && index < head.count;
}

boolean insertLinearAt(LINEAR *head, int index, USER_TYPE value){
	boolean ok = TRUE;
	int i = 0;
	
	if(isLinearFull(*head))
		ok = FALSE;
	else if(FALSE == isIndexValid(*head, index))
		ok = FALSE;
	else{
		//分两步走：
		//1、将index开始向后的所有元素向后移动一个元素空间
		/*循环三要素：
		 *count		<-- count -1
		 *count	-1	<-- count -2
		 *count	-2	<-- count -3
		 *....
		 *index+1	<-- index
		 *初值： i = count ;条件：i > index； 步长：i--
		 */
		for(i = head->count; i > index; i--){
			head->data[i] = head->data[i-1];
		}
		//2、将value写入index处
		head->data[index] = value;
		//3、将count++
		head->count++;
	}
	
	return ok;
}

boolean appendLinear(LINEAR *head, USER_TYPE value){
	boolean ok = TRUE;

	if(TRUE == isLinearFull(*head))
		ok = FALSE;
	else
		head->data[head->count++] = value;

	return ok;	
}

boolean setLinearElementAt(LINEAR head, int index, USER_TYPE value){
	boolean ok = TRUE;

	if(FALSE == isIndexValid(head, index))
		ok = FALSE;
	else
		head.data[index] = value;

	return ok;
}

int getLinearIndex(USER_TYPE data, LINEAR head, int (*cmp)()){
	int i = 0;
	boolean found = FALSE;	

	while(!found && i < head.count)	{
		if(0 == cmp(head.data[i], data))
			found = TRUE;
		else
			i++;

	}
	
	if(!found)
		i = NOT_FOUND;

	return i;
}

boolean getLinearElementAt(USER_TYPE *data, LINEAR head, int index){
	boolean ok = TRUE;

	if(TRUE == isIndexValid(head, index))
		*data = head.data[index];
	else
		ok = FALSE;
	

	return ok;
}

boolean isLinearEmpty(LINEAR head){
	return (0 == head.count);
}

boolean isLinearFull(LINEAR head){
	return head.count >= head.linearMaxRoom;
}

void destoryLinear(LINEAR **head){
	if((*head) == NULL)	return;
	
	if((*head)->data != NULL)
		free((*head)->data);
	free(*head);
}

void initLinear(LINEAR **head,int maxRoom){
	if(*head != NULL)	return;

	(*head) = (LINEAR *)malloc(sizeof(LINEAR));
	(*head)->data = (USER_TYPE *)malloc(sizeof(USER_TYPE));
	(*head)->linearMaxRoom = maxRoom;
	(*head)->count = 0;
}





#endif
