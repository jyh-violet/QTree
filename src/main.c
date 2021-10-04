#include "query/QueryRange.h"
#include "query/QueryMeta.h"
#include "index/QTree.h"
#include <libconfig.h>
#include <time.h>
#include <Tool/ArrayList.h>


#define MaxThread 10

DataRegionType dataRegionType = Zipf;
DataPointType dataPointType = RemovePoint;
int valueSpan = 30; // 2 ^valueSpan
int maxValue ;
int redunc = 5;  // 2^redunc
int coordianteRedunc = 2;  // 2^redunc
SearchKeyType searchKeyType = RAND;
int Qid = 0;
BOOL countFragment = FALSE;
int TOTAL = (int) 100, TRACE_LEN = 100000;
double insertRatio = 0;
u_int64_t checkLeaf = 0;
u_int64_t checkQuery = 0;
u_int64_t checkInternal = 0;
int removePoint = 0;
_Atomic int insertNum = 0, removeNum = 0;
int threadnum = 4;

typedef struct ThreadAttributes{
    QTree* qTree;
    QueryMeta* queries ;
    QueryMeta* removeQuery;
    int start;
    int end;
}ThreadAttributes;

void testInsert(ThreadAttributes* attributes){
    for(    int i = attributes->start; i <  attributes->end; i ++){
        QTreePut(attributes->qTree, &(attributes->queries[i].dataRegion), attributes->queries + i);
    }
}

size_t testMix(ThreadAttributes* attributes){
    Arraylist* removedQuery = ArraylistCreate(TOTAL);
    for (int i = attributes->start; i <  attributes->end; ++i) {
        int randNum = rand();
        double ratio = ((double )randNum) / ((double )RAND_MAX + 1);
        if(ratio < insertRatio){
            QTreePut(attributes->qTree, &(attributes->queries[i].dataRegion), attributes->queries + i);
            insertNum ++;
        } else{
            QTreeFindAndRemoveRelatedQueries(attributes->qTree, (attributes->queries[i].dataRegion.upper + attributes->queries[i].dataRegion.lower) / 2, removedQuery);
            removeNum ++;
        }
    }
    return removedQuery->size;
}

int test() {
#undef BOrder_65
#define BOrder_129
    double generateT = 0, putT = 0, removeT = 0, mixT = 0;
//    TOTAL = 100000;
    srand((unsigned)time(NULL));
    clock_t   start,   finish, time1, time2;
    QTree qTree;
    QTreeConstructor(&qTree, 2);
    QueryMeta* queries = (QueryMeta*)malloc(sizeof(QueryMeta) * TOTAL );
    QueryMeta* removeQuery = (QueryMeta*)malloc(sizeof(QueryMeta) * TOTAL);
    time1 = start = clock();

    for(int i = 0; i < TOTAL ;i ++){
        QueryMetaConstructor(queries + i);
    }
    DataRegionType  dataRegionTypeOld = dataRegionType;
    dataRegionType = Remove;
    for(int i = 0; i < TOTAL;i ++){
        QueryMetaConstructor(removeQuery + i);
    }
    finish = clock();
    generateT = (double)(finish - start)/CLOCKS_PER_SEC;
//    printf("generate end! use %lfs\n", (double)(finish - start)/CLOCKS_PER_SEC );

    int perThread = TOTAL / threadnum;
    time1 = start = clock();
    pthread_t thread[MaxThread];
    ThreadAttributes attributes[MaxThread];
    for (int i = 0; i < threadnum; ++i) {
        attributes[i].start = i * perThread;
        attributes[i].end = i == (threadnum - 1)? TOTAL: (i + 1)* perThread;
        attributes[i].queries = queries;
        attributes[i].removeQuery = removeQuery;
        attributes[i].qTree = &qTree;
        pthread_create(&thread[i], 0, (void *(*)(void *))testInsert, (void *)&attributes[i]);
    }
    for (int i = 0; i < threadnum; ++i) {
        pthread_join(thread[i], NULL);
    }
//    for(    int i = 0; i < TOTAL; i ++){
//        QTreePut(&qTree, &(queries[i].dataRegion), queries + i);
//    }
    finish = clock();
    putT = (double)(finish - start)/CLOCKS_PER_SEC;
    int num = qTree.elements;
    for(int i = 0; i < batchSize; i ++){
        num += qTree.batchCount[i];
    }
    printf("%d\n", num);


    Arraylist* removedQuery = ArraylistCreate(TOTAL);
    time1 = start = clock();

    for (int i = 0; i < threadnum; ++i) {
        attributes[i].start = i * perThread;
        attributes[i].end = i == (threadnum - 1)? TOTAL: (i + 1)* perThread;
        attributes[i].queries = queries;
        attributes[i].removeQuery = removeQuery;
        attributes[i].qTree = &qTree;
        pthread_create(&thread[i], 0, testMix, &attributes[i]);
    }
    size_t removed = 0;
    for (int i = 0; i < threadnum; ++i) {
        size_t removedNum;
        pthread_join(thread[i], &removedNum);
        removed += removedNum;
    }
//    for (int i = 0; i < TOTAL; ++i) {
//        int randNum = rand();
//        double ratio = ((double )randNum) / ((double )RAND_MAX + 1);
//        if(ratio < insertRatio){
//            QTreePut(&qTree, &(queries[i].dataRegion), queries + i);
//            insertNum ++;
//        } else{
//            QTreeFindAndRemoveRelatedQueries(&qTree, (queries[i].dataRegion.upper + queries[i].dataRegion.lower) / 2, removedQuery);
//            removeNum ++;
//        }
//    }
    finish = clock();

//    printf( "get and remove end!\n remain:%d\n",  qTree.elements);
    ArraylistDeallocate(removedQuery);

    QTreeDestroy(&qTree);

    mixT = (double)(finish - start)/CLOCKS_PER_SEC;
//    printf("remove end! use %lfs\n", (double)(finish - start)/CLOCKS_PER_SEC);
//    printf( "get and remove end!\n remain:%d\n",  qTree.elements);
printf("%d, %d, %d, %d, %.2lf, %d,  %d,  %.3lf,%.3lf,%.3lf, %d, %d, %ld, %ld, %ld,  %ld, %ld, %ld, %ld, %ld, %d, %d, %d\n",
           Border, dataPointType, dataRegionTypeOld, searchKeyType, insertRatio, removePoint, TOTAL,
           generateT, putT, mixT, insertNum, removeNum, removed, checkQuery, checkLeaf, checkInternal,
           qTree.leafSplitCount, qTree.internalSplitCount, qTree.whileCount, qTree.funcCount, RemovedQueueSize, batchSize, MaxBatchCount);
    free(queries) ;
    free(removeQuery);
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
    config_lookup_int(&cfg, "TOTAL", &TOTAL);
    config_lookup_int(&cfg, "TRACE_LEN", &TRACE_LEN);
    config_lookup_int(&cfg, "dataRegionType", (int*)&dataRegionType);
    config_lookup_int(&cfg, "dataPointType", (int*)&dataPointType);
    config_lookup_int(&cfg, "valueSpan", &valueSpan);
    config_lookup_float(&cfg, "insertRatio", &insertRatio);
    config_lookup_int(&cfg, "threadnum", &threadnum);

    maxValue = TOTAL;

    test();
    return 0;
}