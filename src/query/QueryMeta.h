//
// Created by jyh_2 on 2021/5/14.
//
#include <stdlib.h>
#include <string.h>
#include "../common.h"
#include "QueryRange.h"


#ifndef QTREE_QUERYMETA_H
#define QTREE_QUERYMETA_H

 int queryCount;
#define queryIdLen 20
typedef struct QueryMeta {
    char    queryId[queryIdLen];
    QueryRange  dataRegion;  //data region of the query

}QueryMeta;

char* getQueryId(QueryMeta* queryMeta);
bool QueryMetaCover(QueryMeta* queryMeta, int value);
void QueryMetaConstructor(QueryMeta* queryMeta);
bool QueryIdCmp(QueryMeta* queryMeta1, QueryMeta* queryMeta2);
void QueryMetaDestroy(QueryMeta* queryMeta);
#endif //QTREE_QUERYMETA_H
