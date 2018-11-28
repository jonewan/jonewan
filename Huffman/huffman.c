//fileName:     huffman.c
//author:       jonewan
//date:         2018/11/27

#include <stdio.h>
#include <malloc.h>
#include <string.h>

typedef struct HUFFMAN{
    char Alpha;
    int Freq;
    int Flag;
    int left;
    int right;
    char *huffmanCode;
}HUFFMAN;

typedef struct FREQ{
    char Alpha;
    int Freq;
}FREQ;

typedef unsigned char   boolean;

#define TRUE    1
#define FALSE   0

FREQ *accountFreq(char *s, int *count);
HUFFMAN *creatHuffmanTable(FREQ *freq, int count);
int findMinFreqIndex(HUFFMAN *huf, int cnt);
void makeHuffmanCode(HUFFMAN *huf, int root, int index, char *str);
void destoryHuffmanTable(HUFFMAN *huf, int count);
char *getHuffmanCode(HUFFMAN *huf, int count, char alpha);
int getHuffCodeMaxLength(HUFFMAN *huf, int count);
char *decodeHuffman(HUFFMAN *huf, char *code, int count);

char *decodeHuffman(HUFFMAN *huf, char *code, int count){
    int i = 0;
    int root = 2 * (count - 1);
    int len = strlen(code);
    char *decode = NULL;
    int decodeStrIndex = 0;

    decode = (char *)malloc(sizeof(char) * (huf[root].Freq + 1));//根结点的字符频度就是整个原字符串的长度 +1给出\0结束标志

    for(i = 0; i <= len; i++){
        if(huf[root].left == -1){//遇到叶子节点
            decode[decodeStrIndex++] = huf[root].Alpha;
            root = 2 * (count - 1);
            i--;
        }
        else if(code[i] == '1')//遇到1去左子树
            root = huf[root].left;
        else//遇到0去右子树
            root = huf[root].right;
    }

    decode[decodeStrIndex] = 0;//加上0结束标志

    return decode;
}

int getHuffCodeMaxLength(HUFFMAN *huf, int count){
    int length = 1;
    int i = 0;

    for(i = 0; i < count ; i++)
        length += huf[i].Freq * strlen(huf[i].huffmanCode);
    
    return length;
}

char *getHuffmanCode(HUFFMAN *huf, int count, char alpha){
    int head = 0;
    int tail = count -1;
    int mid = (head + tail)/2;
    boolean found = FALSE;

    while(head <= tail && found == FALSE){
        mid = (head + tail)/2;
        if(alpha == huf[mid].Alpha)
            found = TRUE;
        else if(alpha > huf[mid].Alpha)
            head = mid + 1;
        else
            tail = mid - 1;
    }

    if(found == FALSE)
        mid = count;

    return huf[mid].huffmanCode;
}

void destoryHuffmanTable(HUFFMAN *huf, int count){
    int i;

    for(i = 0; i < count; i++)
        free(huf[i].huffmanCode);
    free(huf);
}

void makeHuffmanCode(HUFFMAN *huf, int root, int index, char *str){//生成哈夫曼编码
    if(huf[root].left == -1){
        str[index] = 0;
        strcpy(huf[root].huffmanCode, str);
    }
    else{
        str[index] = '1';
        makeHuffmanCode(huf, huf[root].left, index+1, str);
        str[index] = '0';
        makeHuffmanCode(huf, huf[root].right, index+1, str);
    }
}

int findMinFreqIndex(HUFFMAN *huf, int cnt){
    int minIndex = -1;
    int i;

    for(i = 0; i < cnt; i++)
        if(huf[i].Flag == 0 &&(minIndex == -1 || huf[minIndex].Freq > huf[i].Freq))
            minIndex = i;

    huf[minIndex].Flag = 1;

    return minIndex;
}

HUFFMAN *creatHuffmanTable(FREQ *freq, int count){
    HUFFMAN *huf = NULL;
    int i;
    int cnt = count;
    int left, right;

    huf = (HUFFMAN *)calloc(sizeof(HUFFMAN), 2*count -1);
    for(i = 0; i < count; i++){
        huf[i].Alpha = freq[i].Alpha;
        huf[i].Freq = freq[i].Freq;
        huf[i].left = huf[i].right = -1;
        huf[i].huffmanCode = (char *)malloc(sizeof(char) * count);//由n个字符组成的哈夫曼数的最大高度为n-1，加上0结束标志，总申请空间为n
    }

    for(i = 0; i < count -1; i++){
        left = findMinFreqIndex(huf, cnt);
        right = findMinFreqIndex(huf, cnt);
        huf[cnt].Freq = huf[left].Freq + huf[right].Freq;
        huf[cnt].left = left;
        huf[cnt].right = right;
        cnt++;
    }

    return huf;
}

FREQ *accountFreq(char *s, int *count){
    int ascii[256] = {0};
    int cnt = 0;
    FREQ *freq = NULL;
    int i = 0, j = 0;

    for(i = 0; s[i]; i++){
        if(ascii[s[i]] == 0)
            cnt++;
        ascii[s[i]]++;
    }

    freq = (FREQ *)malloc(sizeof(FREQ) * cnt);

    for(i = 0; i < 256; i++){
        if(ascii[i] > 0){
            freq[j].Alpha = i;
            freq[j++].Freq = ascii[i];
        }
    }

    *count = cnt;

    return freq;
}

int main(void){
    char s[256];
    char str[256];
    FREQ *freq = NULL;
    int count;
    int i = 0;
    HUFFMAN *huff;
    char *res = NULL;
    char *decodeStr = NULL;

    puts("请输入一个字符串: ");
    gets(s);

    freq = accountFreq(s, &count);
    for(i = 0; i < count; i++)
        printf("%c\t%d\n", freq[i].Alpha, freq[i].Freq);

    huff = creatHuffmanTable(freq , count);
    makeHuffmanCode(huff, 2*(count-1), 0, str);
    puts("下标\t字符\tASCII\t频度\t标志\t左孩子\t右孩子\t哈夫曼编码\n");
    for(i = 0; i < 2 * count - 1; i++)
        printf("%d\t%c\t%-4d\t%-4d\t%-4d\t%-6d\t%-6d\t%-20s\n",i, huff[i].Alpha == 0 ? ' ': huff[i].Alpha, huff[i].Alpha, huff[i].Freq, huff[i].Flag, huff[i].left, huff[i].right, huff[i].huffmanCode == NULL ? "无" : huff[i].huffmanCode);
    res = (char *)calloc(sizeof(char), getHuffCodeMaxLength(huff, count)+1);
    
    for(i = 0; s[i]; i++)
        strcat(res, getHuffmanCode(huff, count, s[i])) ;
    puts(res);

    decodeStr = decodeHuffman(huff, res, count);

    puts("解码：");
    puts(decodeStr);

    free(decodeStr);
    free(res);
    free(freq);
    destoryHuffmanTable(huff, count);
    return 0;
}