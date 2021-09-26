//
// Created by jyh_2 on 2021/5/14.
//

#ifndef QTREE_TOOLS_H
#define QTREE_TOOLS_H


typedef enum DataRegionType {
    Same,
    Random,
    Increase,
    Zipf,
    Remove
}DataRegionType;

typedef enum DataPointType{
    RemovePoint,
    LowPoint,
    MidPoint
}DataPointType;

typedef enum SearchKeyType{
    LOW,
    DYMID,
    Mid,
    REMOVE,
    RAND
}SearchKeyType;

typedef enum BOOL{
    FALSE,
    TRUE
}BOOL;





int zipf(double alpha, int n);
void printArray(int* array, int num);
void bubbleSort(void* arr[], int len, BOOL (*cmp)(void*, void *));
char *myItoa(int num, char *str);
#endif //QTREE_TOOLS_H
