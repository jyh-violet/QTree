#include <iostream>
#include "query/QueryRange.h"
#include "query/QueryMeta.h"
#include "index/QTree.h"
#include <libconfig.h>



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
    const char ConfigFile[]= "config.cfg";

    config_t cfg;
    config_setting_t *setting;
    const char *str;

    config_init(&cfg);

    /* Read the file. If there is an error, report it and exit. */
    if(! config_read_file(&cfg, ConfigFile))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return(EXIT_FAILURE);
    }
    int regionType, keyType;
    config_lookup_int(&cfg, "TOTAL", &TOTAL);
    config_lookup_int(&cfg, "TRACE_LEN", &TRACE_LEN);
    config_lookup_int(&cfg, "dataRegionType", &regionType);
    config_lookup_int(&cfg, "valueSpan", &valueSpan);
    config_lookup_int(&cfg, "searchKeyType", &keyType);
    switch (regionType) {
        case 0:
            dataRegionType = Same;
            break;
        case 1:
            dataRegionType = Random;
            break;
        case 2:
            dataRegionType = Increase;
            break;
        case 3:
            dataRegionType = Zipf;
            break;
    }
    switch (keyType) {
        case 0:
            searchKeyType = LOW;
            break;
        case 1:
            searchKeyType = DYMID;
            break;
        case 2:
            searchKeyType = RAND;
            break;


    }

//    TOTAL = 10000000;
//    TRACE_LEN = 10000000;
//    dataRegionType = Random;
//    valueSpan = 20;
//    searchKeyType = LOW;
    printf("TOTAL: %d, dataRegionType:%d, valueSpan:%d, searchKeyType:%d \n",
           TOTAL, dataRegionType, valueSpan, searchKeyType);
    for (int i = 0; i < 10; ++i) {
        test();
    }
//    searchKeyType = DYMID;
//    printf("TOTAL: %d, dataRegionType:%d, valueSpan:%d, searchKeyType:%d \n",
//           TOTAL, dataRegionType, valueSpan, searchKeyType);
//    for (int i = 0; i < 10; ++i) {
//        test();
//    }
//    searchKeyType = RAND;
//    printf("TOTAL: %d, dataRegionType:%d, valueSpan:%d, searchKeyType:%d \n",
//           TOTAL, dataRegionType, valueSpan, searchKeyType);
//    for (int i = 0; i < 10; ++i) {
//        test();
//    }
//
//    dataRegionType = Zipf;
//    valueSpan = 16;
//    searchKeyType = LOW;
//    printf("TOTAL: %d, dataRegionType:%d, valueSpan:%d, searchKeyType:%d \n",
//           TOTAL, dataRegionType, valueSpan, searchKeyType);
//    for (int i = 0; i < 10; ++i) {
//        test();
//    }
//    searchKeyType = DYMID;
//    printf("TOTAL: %d, dataRegionType:%d, valueSpan:%d, searchKeyType:%d \n",
//           TOTAL, dataRegionType, valueSpan, searchKeyType);
//    for (int i = 0; i < 10; ++i) {
//        test();
//    }
//    searchKeyType = RAND;
//    printf("TOTAL: %d, dataRegionType:%d, valueSpan:%d, searchKeyType:%d \n",
//           TOTAL, dataRegionType, valueSpan, searchKeyType);
//    for (int i = 0; i < 10; ++i) {
//        test();
//    }

    return 0;
}