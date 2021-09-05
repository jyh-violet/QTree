//
// Created by jyh_2 on 2021/5/14.
//

#ifndef QTREE_TOOLS_H
#define QTREE_TOOLS_H


typedef enum DataRegionType {
    Same,
    Random,
    Increase,
    Zipf
}DataRegionType;

typedef enum SearchKeyType{
    LOW,
    DYMID,
    RAND
}SearchKeyType;

typedef enum bool{
    false,
    true
}bool;





int zipf(double alpha, int n);
void printArray(int* array, int num);
void bubbleSort(void* arr[], int len, bool (*cmp)(void*, void *));
char *myItoa(int num, char *str);
#endif //QTREE_TOOLS_H
