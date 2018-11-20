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
int getLinearIndex(USER_TYPE data, LINEAR head, int (*cmp)(USER_TYPE, USER_TYPE));
boolean setLinearElementAt(LINEAR head, int index, USER_TYPE value);
boolean appendLinear(LINEAR *head, USER_TYPE value);
boolean insertLinearAt(LINEAR *head, int index, USER_TYPE value);
boolean isIndexValid(LINEAR head, int index);
int getLinearElementCount(LINEAR head);
int getLinearMaxRoom(LINEAR head);
boolean removeLinearElementAt(LINEAR *head, int index);
boolean removeLinearEnement(LINEAR *head, USER_TYPE value, int (*cmp)());


#endif
