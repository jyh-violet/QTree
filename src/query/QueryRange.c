//
// Created by jyh_2 on 2021/6/18.
//

#include "QueryRange.h"

static int rangeCount;
extern  DataRegionType dataRegionType;
extern  DataPointType dataPointType;
extern int maxValue;
extern int removePoint;


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
            switch (dataPointType) {
                case RemovePoint:
                    queryRange->lower = r + span - diff / 4 * 3;
                    queryRange->upper = r + span + diff / 4 ;
                    break;
                case LowPoint:
                    queryRange->lower = r + span ;
                    queryRange->upper = r + span + diff ;
                    break;
                case MidPoint:
                    queryRange->lower = r + span - diff / 2;
                    queryRange->upper = r + span + diff / 2;
                    break;
            }

            break;
        case Zipf:
            randNum = zipf(0.99, maxValue);
            diff = (int) RAND_RANGE(span);
            switch (dataPointType) {
                case RemovePoint:
                    queryRange->lower = randNum + span - diff / 4 * 3;
                    queryRange->upper = randNum + span + diff / 4 ;
                    break;
                case  LowPoint:
                    queryRange->lower = randNum + span ;
                    queryRange->upper = randNum + span + diff ;
                    break;
                case  MidPoint:
                    queryRange->lower = randNum + span - diff / 2;
                    queryRange->upper = randNum + span + diff / 2;
                    break;
            }
            break;
        case Increase:
            queryRange->lower = rangeCount ++ ;
            queryRange->upper = rangeCount ++ ;
            break;
        case Remove:
            randNum = zipf(0.99, maxValue);
            queryRange->lower =  removePoint  + randNum + span ;
            queryRange->upper =  removePoint  + randNum + span;
            break;
    }
//    queryRange->boundInclude = 3;
//    queryRange->searchKey  = -1;
}

void QueryRangeConstructorWithPara(QueryRange *queryRange, BoundKey startKey, BoundKey endKey, BOOL lowerInclude, BOOL upperInclude){
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
//    queryRange->searchKey = -1;
}

void printQueryRange(QueryRange* queryRange){
    printf("{%d,%d}", queryRange->lower, queryRange->upper);
}