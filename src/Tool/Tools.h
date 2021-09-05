//
// Created by jyh_2 on 2021/5/14.
//

#ifndef QTREE_TOOLS_H
#define QTREE_TOOLS_H


enum DataRegionType {
    Same,
    Random,
    Increase,
    Zipf
};

typedef enum SearchKeyType{
    LOW,
    DYMID,
    RAND
}SearchKeyType;



static int TOTAL = (int) 10000000, TRACE_LEN = 100000000;

int zipf(double alpha, int n);
void printArray(int* array, int num);

#endif //QTREE_TOOLS_H
