//
// Created by jyh_2 on 2021/6/18.
//

#include "QueryRange.h"

static int rangeCount;
extern  DataRegionType dataRegionType;
extern int maxValue;
extern int span;



void QueryRangeConstructor(QueryRange *queryRange){

    int r, diff, randNum;

    switch (dataRegionType){
        case Same:
            queryRange->lower = rangeCount;
            queryRange->upper = rangeCount + 1;
            break;
        case  Random:
            r = (int) RAND_RANGE(maxValue);
            diff = (int) RAND_RANGE(span);
            if(r > diff / 2){
                queryRange->lower = r - diff / 2;
                queryRange->upper = r + diff / 2;
            } else{
                queryRange->lower = r ;
                queryRange->upper = r + diff ;
            }
            break;
        case Zipf:
            randNum = zipf(0.99, maxValue);
            diff = (int) RAND_RANGE(span);
            if(randNum > diff /2){
                queryRange->lower =  randNum - diff / 2;
                queryRange->upper =  randNum  + diff / 2;
            } else{
                queryRange->lower =  randNum;
                queryRange->upper =  randNum  + diff;
            }

            break;
        case Increase:
            queryRange->lower = rangeCount ++ ;
            queryRange->upper = rangeCount ++ ;
            break;
    }
//    queryRange->boundInclude = 3;
    queryRange->searchKey  = -1;
}

void QueryRangeConstructorWithPara(QueryRange *queryRange, int startKey, int endKey, BOOL lowerInclude, BOOL upperInclude){
    queryRange->lower = startKey;
    queryRange->upper = endKey;
//    if(lowerInclude && upperInclude){
//        queryRange->boundInclude = 3;
//    }else if (lowerInclude) {
//        queryRange->boundInclude = 1;
//    }else if (upperInclude){
//        queryRange->boundInclude = 2;
//    }else{
//        queryRange-> boundInclude = 0;
//    }
    queryRange->searchKey = -1;
}

void printQueryRange(QueryRange* queryRange){
    printf("{%d,%d}", queryRange->lower, queryRange->upper);
}