#include <iostream>
#include "query/QueryRange.h"
#include "query/QueryMeta.h"
#include "index/QTree.h"



int test() {

    srand((unsigned)time(NULL));
    clock_t   start,   finish, time1, time2;
    QTree<QueryRange, QueryMeta> qTree(32);
    list<QueryMeta*>* queries = new list<QueryMeta*>();
//    QueryMeta* queries[TOTAL];
    time1 = start = clock();

//    for (int i = 0; i < 20; ++i) {
//        cout << zipf(0.99, 30) << endl;
//    }
//
//    return 0;
    for(int i = 0; i < TOTAL;i ++){
        QueryMeta* queryMeta = new QueryMeta();
        queries->push_back(queryMeta);
        if((i + 1) % TRACE_LEN == 0){
            time2 = clock();
            cout << "generate " << TRACE_LEN << " use " << (double)(time2 - time1)/CLOCKS_PER_SEC << "s" << endl;
            time1 = time2;
        }
    }
    finish = clock();
    cout << "generate end! use " << (double)(finish - start)/CLOCKS_PER_SEC << "s" << endl;


    list<QueryMeta*>::iterator  itreator = queries->begin();
    int i = 0;
    time1 = start = clock();
    for(itreator = queries->begin(); itreator!= queries->end(); itreator ++){
        qTree.put((*itreator)->dataRegion, (*itreator));
        if((i + 1) % TRACE_LEN == 0){
            time2 = clock();
            cout << "put " << TRACE_LEN << " use " << (double)(time2 - time1)/CLOCKS_PER_SEC << "s" << endl;
            time1 = time2;
        }
        i ++;
    }
    finish = clock();
    cout << "put end! use " << (double)(finish - start)/CLOCKS_PER_SEC << "s" << endl;
    //    cout << &qTree << endl;


    list<QueryMeta*>* removedQuery = new list<QueryMeta*>();
    time1 = start = clock();

    i = 0;
    for(itreator = queries->begin(); itreator!= queries->end(); itreator ++){
        qTree.findAndRemoveRelatedQueries((*itreator)->dataRegion->lower, removedQuery);
//        list <QueryMeta*> :: iterator it;
//        for(it = removedQuery->begin(); it!=removedQuery->end(); it ++){
//            cout << (*it)->getQueryId() << " ";
//        }
//        cout << endl;
        if((i + 1) % TRACE_LEN == 0){
            time2 = clock();
            cout << "remove " << TRACE_LEN << " use " << (double)(time2 - time1)/CLOCKS_PER_SEC << "s" << endl;
            time1 = time2;
        }
        i ++;
    }
    finish = clock();
    cout << "remove end! use " << (double)(finish - start)/CLOCKS_PER_SEC << "s" << endl;

    cout << "get and remove end!" << endl;
    cout << "remain:" << qTree.elements << endl;
//    cout << &qTree << endl;

    for(itreator = queries->begin(); itreator!= queries->end(); itreator ++){
        delete (*itreator);
    }
//    cout << &qTree << endl;

    delete queries;
    return 0;
}

int main(){
    TOTAL = 200000000;
    TRACE_LEN = 200000000;
    dataRegionType = Random;
    valueSpan = 20;
    searchKeyType = RAND;
    for (int i = 0; i < 10; ++i) {
        test();
    }
    searchKeyType = DYMID;
    for (int i = 0; i < 10; ++i) {
        test();
    }
    searchKeyType = RAND;
    for (int i = 0; i < 10; ++i) {
        test();
    }

    dataRegionType = Zipf;
    valueSpan = 16;
    searchKeyType = RAND;
    for (int i = 0; i < 10; ++i) {
        test();
    }
    searchKeyType = DYMID;
    for (int i = 0; i < 10; ++i) {
        test();
    }
    searchKeyType = RAND;
    for (int i = 0; i < 10; ++i) {
        test();
    }

    return 0;
}