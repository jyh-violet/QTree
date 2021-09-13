#include "query/QueryRange.h"
#include "query/QueryMeta.h"
#include "index/QTree.h"
#include <libconfig.h>
#include <time.h>
#include <Tool/ArrayList.h>

DataRegionType dataRegionType = Zipf;
int valueSpan = 30; // 2 ^valueSpan
int maxValue ;
int span = 10000;
int redunc = 5;  // 2^redunc
int coordianteRedunc = 2;  // 2^redunc
SearchKeyType searchKeyType = RAND;
int Qid = 0;
BOOL countFragment = FALSE;
int removeNum = 10;
int TOTAL = (int) 100, TRACE_LEN = 100000;
double insertRatio = 0;

int test() {
#undef BOrder_65
#define BOrder_129
    double generateT, putT, removeT, mixT;
    srand((unsigned)time(NULL));
    clock_t   start,   finish, time1, time2;
    QTree qTree;
    QTreeConstructor(&qTree, 2);
    QueryMeta* queries = (QueryMeta*)malloc(sizeof(QueryMeta) * TOTAL * 2);
    time1 = start = clock();

    for(int i = 0; i < TOTAL * 2;i ++){
        QueryMetaConstructor(queries + i);
        if((i + 1) % TRACE_LEN == 0){
            time2 = clock();
            printf("generate %d use %lfs\n", TRACE_LEN, (double)(time2 - time1)/CLOCKS_PER_SEC );
            time1 = time2;
        }
    }
    finish = clock();
    generateT = (double)(finish - start)/CLOCKS_PER_SEC;
//    printf("generate end! use %lfs\n", (double)(finish - start)/CLOCKS_PER_SEC );

    int i = 0;
    time1 = start = clock();
    for(; i < TOTAL; i ++){
        QTreePut(&qTree, &(queries[i].dataRegion), queries + i);
        if((i + 1) % TRACE_LEN == 0){
            time2 = clock();
            printf("put %d use %lfs\n", TRACE_LEN, (double)(time2 - time1)/CLOCKS_PER_SEC );
            time1 = time2;
        }
    }
    finish = clock();
    putT = (double)(finish - start)/CLOCKS_PER_SEC;
    Arraylist* removedQuery = ArraylistCreate(TOTAL);

    time1 = start = clock();
    int insertNum = 0, removeNum = 0;
    for (int i = 0; i < TOTAL; ++i) {
        int randNum = rand();
        double ratio = ((double )randNum) / ((double )RAND_MAX + 1);
        if(ratio < insertRatio){
            QTreePut(&qTree, &(queries[i + TOTAL].dataRegion), queries + i + TOTAL);
            insertNum ++;
        } else{
            QTreeFindAndRemoveRelatedQueries(&qTree, queries[i + TOTAL].dataRegion.upper, removedQuery);
            removeNum ++;
        }
    }
    finish = clock();
    size_t removed = removedQuery->size;
//    printf( "get and remove end!\n remain:%d\n",  qTree.elements);
    ArraylistDeallocate(removedQuery);
    QTreeDestroy(&qTree);

    mixT = (double)(finish - start)/CLOCKS_PER_SEC;
//    printf("remove end! use %lfs\n", (double)(finish - start)/CLOCKS_PER_SEC);
//    printf( "get and remove end!\n remain:%d\n",  qTree.elements);
    printf("%lf,%lf,%lf, %d, %d, %ld\n", generateT, putT, mixT, insertNum, removeNum, removed);
    free(queries) ;
    return 0;
}

int main(){

    const char ConfigFile[]= "config.cfg";

    config_t cfg;

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
    config_lookup_float(&cfg, "insertRatio", &insertRatio);
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
            searchKeyType = Mid;
            break;
        case 3:
            searchKeyType = RAND;
            break;


    }

//    maxValue = 1 << (valueSpan - 1);
    maxValue = TOTAL;
    span = maxValue >> 1;

//    TOTAL = 10000000;
//    TRACE_LEN = 10000000;
//    dataRegionType = Random;
//    valueSpan = 20;
//    searchKeyType = LOW;
//    printf("TOTAL: %d, TRACE_LEN:%d, dataRegionType:%d, valueSpan:%d, searchKeyType:%d \n",
//           TOTAL, TRACE_LEN, dataRegionType, valueSpan, searchKeyType);
//    for (int i = 0; i < 10; ++i) {
//        test();
//    }
    test();
    return 0;
}