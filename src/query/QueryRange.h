//
// Created by jyh_2 on 2021/6/18.
//

#ifndef QTREE_QUERYRANGE_H
#define QTREE_QUERYRANGE_H

#include "../common.h"

#define QueryRangeLT(queryRange1, queryRange2) \
(BOOL)(((QueryRange*)queryRange1)->searchKey < ((QueryRange*)queryRange2)->searchKey)

#define QueryRangeGT(queryRange1, queryRange2)  \
(BOOL)(((QueryRange*)queryRange1)->searchKey > ((QueryRange*)queryRange2)->searchKey)

#define QueryRangeMaxGE(queryRange1, queryRange2)  \
(BOOL)(((QueryRange* )queryRange1)->upper >= ((QueryRange* )queryRange2)->upper)

#define QueryRangeMinGT(queryRange1, queryRange2) \
(BOOL)(((QueryRange* )queryRange1)->lower > ((QueryRange*)queryRange2)->lower)

#define QueryRangeCover(queryRange, value)  \
(BOOL)(((QueryRange *)(queryRange))->lower <= (int)value && ((QueryRange *)(queryRange))->upper >= (int)value)

typedef struct QueryRange{
    int lower; // lower bound of the region
    int upper;  // upper bound of the region
    int searchKey; // in [lower, upper], used as the search key in the tree
    char boundInclude ; //0: neither include; 1 :lower include; 2: upper include; 3: both include
}QueryRange;


void QueryRangeConstructor(QueryRange *queryRange);
void QueryRangeConstructorWithPara(QueryRange *queryRange, int startKey, int endKey, BOOL lowerInclude, BOOL upperInclude);
void printQueryRange(QueryRange* queryRange);
#endif //QTREE_QUERYRANGE_H
