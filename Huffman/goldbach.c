//fileName:     goldBach.c
//author:        jonewan
//date:           2018/11/28
//discription:    歌德巴赫猜想：任意一个偶数可以分解为两个质数之和。
//使用位运算来验证1000000以内的数

#include <stdio.h>
#include <math.h>
#include <time.h>

typedef unsigned char boolean;
#define MAX_NUM     1000000

#define SET              1
#define UNSET          0

unsigned char primeArr[MAX_NUM >> 3] = {0};

void setArrVal(int index, boolean value);
boolean getArrVal(int index);
boolean isPrime(int index);
void constructPrimeArr(void);
void goldBachOperation(void);

void goldBachOperation(void){
    int num;
    int i;

    for(num = 6; num < MAX_NUM; num += 2)
        for(i = 3; i <= num/2; i += 2)
            if(isPrime(i) && isPrime(num-i)){
                printf("%d = %d + %d\n", num, i, num-i);
                i = num;//停止当前内层循环
            }
}

void constructPrimeArr(void){
    int i, j;

    for(i = 4; i < MAX_NUM; i += 2)
        setArrVal(i, 1);
    for(i = 3; i <= sqrt(MAX_NUM); i += 2)
        if(isPrime(i))
            for(j = i * i; j <=  MAX_NUM; j += i)
                setArrVal(j, 1);
}
boolean isPrime(int index){
    return !(primeArr[index >> 3] & (128 >> (index & 0x07)));
}

boolean getArrVal(int index){
    return !!(primeArr[index >> 3] & (128 >> (index & 0x07)));
}

void setArrVal(int index, boolean value){
    if(value == 1)
        primeArr[index >> 3] |= (128 >> (index & 0x07));
    else
        primeArr[index >> 3] &= ~(128 >> (index & 0x07));
}

int main(void){
    long startTime = clock();
    long endTime;
    int i;

    constructPrimeArr();
    goldBachOperation();
    endTime = clock();
    printf("use time: %lfs\n", (endTime - startTime)/100000.0);
    return 0;
}