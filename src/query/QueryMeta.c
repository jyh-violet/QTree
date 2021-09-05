//
// Created by jyh_2 on 2021/5/14.
//

#include "QueryMeta.h"


char* getQueryId(QueryMeta* queryMeta){
    return queryMeta->queryId;
}

bool QueryMetaCover(QueryMeta* queryMeta, int value){
    return  QueryRangeCover(&queryMeta->dataRegion, (value));
}
void QueryMetaConstructor(QueryMeta* queryMeta){
    queryCount++;

    myItoa( queryCount, queryMeta->queryId);
    QueryRangeConstructor(&queryMeta->dataRegion);

}
bool QueryIdCmp(QueryMeta* queryMeta1, QueryMeta* queryMeta2){
    return strcmp(queryMeta1->queryId, queryMeta2->queryId);
}

void QueryMetaDestroy(QueryMeta* queryMeta){
}