//fileName:     huffman.c
//author:       jonewan
//date:         2018/11/27

#include <stdio.h>
#include <malloc.h>

typedef struct HUFFMAN{
    char Alpha;
    int Freq;
    int Flag;
    int left;
    int right;
}HUFFMAN;

typedef struct FREQ{
    char Alpha;
    int Freq;
}FREQ;

FREQ *accountFreq(char *s, int *count);
HUFFMAN *creatHuffmanTable(FREQ *freq, int count);
int findMinFreqIndex(HUFFMAN *huf, int cnt);

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
    FREQ *freq = NULL;
    int count;
    int i = 0;
    HUFFMAN *huff;

    puts("请输入一个字符串: ");
    gets(s);

    freq = accountFreq(s, &count);
    for(i = 0; i < count; i++)
        printf("%c\t%d\n", freq[i].Alpha, freq[i].Freq);

    huff = creatHuffmanTable(freq , count);
    puts("字符\t频度\t标志\t左孩子\t右孩子\n");
    for(i = 0; i < 2 * count - 1; i++)
        printf("%c\t%-4d\t%-4d\t%-6d\t%-6d\n",huff[i].Alpha, huff[i].Freq, huff[i].Flag, huff[i].left, huff[i].right);

    return 0;
}