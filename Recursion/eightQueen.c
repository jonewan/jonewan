//fileName:	eightQueen.c
//author:	jonewan
//date:		2018/11/21


#include <stdio.h>

#define RANGE	8 
#define TRUE	1
#define	FALSE	0

void showEightQueen(int (*eightQueen)[RANGE]);
int positionIsSafe(int (*eightQueen)[RANGE], int row, int col);
void dealEightQueen(int (*eightQueen)[RANGE], int row);

void dealEightQueen(int (*eightQueen)[RANGE], int row){
	int col = 0;

	if(row >= RANGE)
		showEightQueen(eightQueen);
	else{
		for(col = 0; col < RANGE; col++){
			if(TRUE == positionIsSafe(eightQueen, row, col)){
				eightQueen[row][col] = 1;
				dealEightQueen(eightQueen,row+1);
				eightQueen[row][col] = 0;
			}
		}
	}


}

int positionIsSafe(int (*eightQueen)[RANGE], int row, int col){//判断放置的位置是否安全
	int ok = TRUE;
	int i, j;

	for(i = row-1, j = col-1; ok && i >= 0 && j >= 0; i--, j--)//先检测左上方是否有皇后
		if(1 == eightQueen[i][j])
			ok = FALSE;

	for(i = row-1, j = col; ok && i >= 0; i--)//检测正上方是否有皇后
		if(1 == eightQueen[i][j])
			ok = FALSE;
	
	for(i = row-1, j = col+1; ok && i >= 0 && j < RANGE; i--, j++)//检测右上方是否有皇后
		if(1 == eightQueen[i][j])
			ok = FALSE;

	return ok;
}

void showEightQueen(int (*eightQueen)[RANGE]){//显示八皇后棋盘
	int row = 0;
	int col = 0;
	static unsigned int count = 1;

	printf("第%d种解:\n", count++);
	for(row = 0; row < RANGE; row++){
		for(col = 0; col < RANGE; col++)
			printf("%4d", eightQueen[row][col]);
		puts("\n");
	}
}

int main(void){
	int eightQueen[RANGE][RANGE] = {0};
	
	dealEightQueen(eightQueen, 0);	

	return 0;
}
