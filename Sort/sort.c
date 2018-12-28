#include "sort.h"

FUN_SORT sort_name[] = {
    insertSort,
    chooseSort,
    swapSort,
    ShellSort,
    heapSort,
    quickSort 
};

static void ShellInnerSort(int *data, int count, int step, int start);
static void adjustHeap(int *data, int count, int root);
static int onceQuickSort(int *data, int start, int end);
static void innerQuickSort(int *data, int start, int end);

void sort(int *data, int count, int sortType){
    if(sortType < 0 || sortType >= sizeof(sort_name)/sizeof(FUN_SORT))
        return ;
    sort_name[sortType](data, count);
}

void quickSort(int *data, int count){
    innerQuickSort(data, 0, count-1);
}

static void innerQuickSort(int *data, int start, int end){
    int middle;
    
    if(start >= end){
        return ;
    }
    
    middle = onceQuickSort(data, start, end);
    innerQuickSort(data, start, middle-1);
    innerQuickSort(data, middle+1, end);
}

static int onceQuickSort(int *data, int start, int end){
    int tmp = data[start];

    while(start < end){
        while(start < end && data[end] >= tmp){
            --end;
        }
        if(start < end){
            data[start] = data[end];
        }
        while(start < end && data[start] <= tmp){
            ++start;
        }
        if(start < end){
            data[end] = data[start];
        }
    }
    data[start] = tmp;

    return start;
}

void heapSort(int *data, int count){
    int root;
    int tmp;


    for(root = count/2 - 1; root > 0; root--){//这里并没有完全生成大根堆，而是除了根结点未调整外都调整了，
        adjustHeap(data, count, root);
    }
    for(; count > 0; count--){
        adjustHeap(data, count, 0);//只针对根结点的调整
        //将根结点与最后一个叶子节点进行交换
        tmp = data[0];
        data[0] = data[count-1];
        data[count-1] = tmp;
    }

}

static void adjustHeap(int *data, int count, int root){
    int maxIndex;//最大元素下标
    int childMaxIndex;//孩子中最大者下标
    int rightIndex;//右孩子下标
    int leftIndex;//左孩子下标
    int tmp;

    while(root <= count/2-1){//判断该数是否为非叶子节点
        leftIndex = 2*root + 1;
        rightIndex = leftIndex + 1;
        //childMaxIndex = 是否只有左孩子节点 ？左孩子下标 ：（右孩子元素>左孩子元素 ？ 右孩子下标：左孩子下标） 
        childMaxIndex = rightIndex >= count ? leftIndex : (data[rightIndex] > data[leftIndex] ? rightIndex : leftIndex);//求得孩子中最大者下标
        //maxIndex = data[root] > 孩子中最大者下标 ？ root ：孩子中最大者下标
        maxIndex = data[root] > data[childMaxIndex] ? root: childMaxIndex;//求得树中最大元素下标

        if(maxIndex == root){
            return ;
        }

        tmp = data[root];
        data[root] = data[maxIndex];
        data[maxIndex] = tmp;
        root = maxIndex;
    }
}

void ShellSort(int *data, int count){
    int step = count;
    int start = 0;

    step |= step >> 1;
    step |= step >> 2;
    step |= step >> 4;
    step |= step >> 8;
    step |= step >> 16;

    for(step = step >> 1; step > 0; step >>= 1){
        for(start = 0; start < step; start++){
            ShellInnerSort(data, count, step, start);
        }
    }
}

void insertSort(int *data, int count){
    int first;
    int i;
    int j;
    int t;

    for(i = 1; i < count; i++){
        first = data[i];
        for(j = 0; j < i && data[j] <= first; j++)
            ;
        for(t = i; t > j; t--){
            data[t] = data[t-1];
        }
        data[t] = first;
    }
}

void chooseSort(int *data, int count){
    int index;
    int minIndex;
    int i;
    int tmp;

    for(index = 0; index < count - 1; index++){
        for(minIndex = i = index; i < count; i++){
            if(data[minIndex] > data[i]){
                minIndex = i;
            }
        }
        if(minIndex != index){
            tmp = data[index];
            data[index] = data[minIndex];
            data[minIndex] = tmp;
        }
    }
}

void swapSort(int *data, int count){
    int i, j, tmp;
    boolean swapFlag = TRUE;

    for(i = 0; swapFlag && i < count; i++){
        for(swapFlag = FALSE, j = 0; j < count-1-i; j++){
            if(data[j] > data[j+1]){
                tmp = data[j+1];
                data[j+1] = data[j];
                data[j] = tmp;
                swapFlag = TRUE;
            }
        }
    }
}
static void ShellInnerSort(int *data, int count, int step, int start){
    int first;
    int i;
    int j;
    int t;

    for(i = start + step; i < count; i += step){
        first = data[i];
        for(j = start; j < i && data[j] <= first; j+=step)
            ;
        for(t = i; t > j; t-=step){
            data[t] = data[t-step];
        }
        data[t] = first;
    }
}
