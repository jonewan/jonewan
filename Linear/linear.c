//fileName:	linear.c
//author:	jonewan
//date:		2018/11/19

#include "./linear.h"
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

int getLinearIndex(USER_TYPE data, LINEAR head, int (*cmp)(USER_TYPE, USER_TYPE)){
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
	*head = NULL;
}

void initLinear(LINEAR **head,int maxRoom){
	if(*head != NULL)	return;

	(*head) = (LINEAR *)malloc(sizeof(LINEAR));
	(*head)->data = (USER_TYPE *)malloc(sizeof(USER_TYPE));
	(*head)->linearMaxRoom = maxRoom;
	(*head)->count = 0;
}


