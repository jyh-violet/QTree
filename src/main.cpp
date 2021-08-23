#include <iostream>
#include "query/QueryRange.h"
#include "query/QueryMeta.h"
#include "index/QTree.h"


int TOTAL = (int) 1000000, TRACE_LEN = 100000;

int main() {

    srand((unsigned)time(NULL));
    clock_t   start,   finish, time1, time2;
    std::cout << "Hello, World!" << std::endl;
    QTree<QueryRange, QueryMeta> qTree(32);
    QueryMeta* queries[TOTAL];
    time1 = start = clock();
    for(int i = 0; i < TOTAL; i ++){
        queries[i] = new QueryMeta();
        qTree.put(queries[i]->dataRegion, (queries[i]));
        if((i + 1) % TRACE_LEN == 0){
            time2 = clock();
            cout << "put " << TRACE_LEN << " use " << (double)(time2 - time1)/CLOCKS_PER_SEC << "s" << endl;
            time1 = time2;
        }
    }
    finish = clock();
    cout << "put end! use " << (double)(finish - start)/CLOCKS_PER_SEC << "s" << endl;
//    cout << &qTree << endl;

    list<QueryMeta*>* removedQuery = new list<QueryMeta*>();
    time1 = start = clock();
    for(int i = 0; i < TOTAL; i ++){
        qTree.findAndRemoveRelatedQueries(queries[i]->dataRegion->lower, removedQuery);
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
    }
    finish = clock();
    cout << "remove end! use " << (double)(finish - start)/CLOCKS_PER_SEC << "s" << endl;

    cout << "get and remove end!" << endl;
    cout << "remain:" << qTree.elements << endl;

    for(int i = 0; i < TOTAL; i ++){
        delete queries[i];
    }
//    cout << &qTree << endl;

    return 0;
}
