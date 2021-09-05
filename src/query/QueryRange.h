//
// Created by jyh_2 on 2021/6/18.
//

#ifndef QTREE_QUERYRANGE_H
#define QTREE_QUERYRANGE_H

#include "../common.h"



typedef struct QueryRange{
    int lower; // lower bound of the region
    int upper;  // upper bound of the region
    int searchKey; // in [lower, upper], used as the search key in the tree
    char boundInclude ; //0: neither include; 1 :lower include; 2: upper include; 3: both include
}QueryRange;


bool QueryRangeCover(QueryRange *queryRange, int value);
void QueryRangeConstructor(QueryRange *queryRange);
void QueryRangeConstructorWithPara(QueryRange *queryRange, int startKey, int endKey, bool lowerInclude, bool upperInclude);
void printQueryRange(QueryRange* queryRange);
bool QueryRangeLT(QueryRange* queryRange1, QueryRange* queryRange2);

bool QueryRangeGT(QueryRange* queryRange1, QueryRange* queryRange2);
bool QueryRangeMaxGE(QueryRange* queryRange1, QueryRange* queryRange2);
bool QueryRangeMinGT(QueryRange* queryRange1, QueryRange* queryRange2);
#endif //QTREE_QUERYRANGE_H
