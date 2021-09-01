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

static DataRegionType dataRegionType = Zipf;
static int valueSpan = 16; // 2 ^valueSpan
static int maxValue = 1 << (valueSpan - 1);
static int span = 1000;
static int redunc = 5;  // 2^redunc
static int coordianteRedunc = 2;  // 2^redunc
static int BOrder = 32;
static SearchKeyType searchKeyType = DYMID;

static int TOTAL = (int) 10000000, TRACE_LEN = 10000000;

int zipf(double alpha, int n);

#endif //QTREE_TOOLS_H
