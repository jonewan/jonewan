#include <stdio.h>

int main(void){
	FILE *fpin;
	FILE *fpout;
	char sourceFileName[128] = "abc.txt";
	int a, b, c, d;

	fpin = fopen(sourceFileName, "r");
	if(fpin != NULL){
		fscanf(fpin, "%d%d%d%d", &a, &b, &c, &d);
		//printf("a=%d b=%d c=%d d=%d", a, b, c, d);
		fclose(fpin);
	}
	fpout = fopen("xyz.txt", "w");//以w方式打开一个并不存在的文件，则，C系统将先行创建这个文件；若这个文件已经存在，则，C系统将先行清空这个文件的所有数据（内容）。
	if(fpout != NULL){
		fprintf(fpout, "a=%d b=%d c=%d d=%d\n", a, b, c, d);
		fwrite
		fclose(fpout);
	}

	return 0;
}
