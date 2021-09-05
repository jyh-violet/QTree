//
// Created by jyh_2 on 2021/5/14.
//

#include "../common.h"
#include "QueryRange.h"


#ifndef QTREE_QUERYMETA_H
#define QTREE_QUERYMETA_H

static int queryCount;

class QueryMeta {
public:
    string    queryId;
    QueryRange  *dataRegion;  //data region of the query
//    RangeHolder *splitHilbertRanges;
//

    QueryMeta(){
        queryCount++;
        queryId = to_string ( queryCount);
        dataRegion = new QueryRange();

    }

    ~QueryMeta(){
        delete this->dataRegion;
    }

    string getQueryId();

    bool cover(int value){
        return  this->dataRegion->cover(value);
    }


};


#endif //QTREE_QUERYMETA_H
