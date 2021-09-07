//
// Created by jyh_2 on 2021/5/14.
//
#include <stdlib.h>
#include <string.h>
#include "../common.h"
#include "QueryRange.h"


#ifndef QTREE_QUERYMETA_H
#define QTREE_QUERYMETA_H

static int queryCount;
#define queryIdLen 30
typedef struct QueryMeta {
    char    queryId[queryIdLen];
    QueryRange  dataRegion;  //data region of the query

}QueryMeta;

char* getQueryId(QueryMeta* queryMeta);
BOOL QueryMetaCover(QueryMeta* queryMeta, int value);
void QueryMetaConstructor(QueryMeta* queryMeta);
void QueryMetaConstructorWithPara(QueryMeta* queryMeta, char* id, int lower, int upper);
BOOL QueryIdCmp(void *queryMeta1, void * queryMeta2);
void QueryMetaDestroy(QueryMeta* queryMeta);
void printQueryMeta(QueryMeta* queryMeta);
#endif //QTREE_QUERYMETA_H
