#include "query/QueryRange.h"
#include "query/QueryMeta.h"
#include "index/QTree.h"
#include <libconfig.h>
#include <time.h>
#include <Tool/ArrayList.h>
#include <papi.h>
#include <pthread.h>

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
double zipfPara = 0.99;
BOOL qtreeCheck = FALSE;

_Atomic int insertNum = 0, removeNum = 0;
int threadnum = 4;

typedef struct TaskRes{
    double usedTime;
    size_t size;
}TaskRes;
typedef struct ThreadAttributes{
    QTree* qTree;
    QueryMeta* insertQueries ;
    QueryMeta* queries ;
    QueryMeta* removeQuery;
    double *mixPara;
    int start;
    int end;
    TaskRes  result;
}ThreadAttributes;

void testInsert(ThreadAttributes* attributes){
    clock_t   start,   finish;
    start = clock();
    for(    int i = attributes->start; i <  attributes->end; i ++){
        QTreePut(attributes->qTree, &(attributes->insertQueries[i].dataRegion), attributes->insertQueries + i);
//        if((i + 1) % 100000 == 0){
//            vmlog(InsertLog,"insert:%d", i);
//        }
    }
    finish = clock();
    attributes->result.usedTime = (double)(finish - start)/CLOCKS_PER_SEC;
    attributes->result.size = attributes->end - attributes->start;
}

void testMix(ThreadAttributes* attributes){
    Arraylist* removedQuery = ArraylistCreate(attributes->end - attributes->start);
    clock_t   start,   finish;
    start = clock();
    for (int i = attributes->start; i <  attributes->end; ++i) {
//        vmlog(MiXLog,"i:%d, para:%lf, rm:%ld",i, attributes->mixPara[i], removedQuery->size);
        if(attributes->mixPara[i] < insertRatio){
            QTreePut(attributes->qTree, &(attributes->queries[i].dataRegion), attributes->queries + i);
            insertNum ++;
        } else{
            QTreeFindAndRemoveRelatedQueries(attributes->qTree, (attributes->removeQuery[i].dataRegion.upper + attributes->removeQuery[i].dataRegion.lower) / 2, removedQuery);
            removeNum ++;
        }
    }
    finish = clock();
    attributes->result.usedTime = (double)(finish - start)/CLOCKS_PER_SEC;
    attributes->result.size = removedQuery->size;
    ArraylistDeallocate(removedQuery);
}

int test() {

    useBFPRT = 0;
    double generateT = 0, putT = 0,  mixT = 0;
//    TOTAL = 1000;
    TRACE_LEN = 1000;
    srand((unsigned)time(NULL));
    initZipfParameter(TOTAL, zipfPara);
    clock_t   start,   finish;
    QTree qTree;
    QTreeConstructor(&qTree, 2);
    double *mixPara = (double *) malloc(sizeof (double ) * TOTAL);
    for (int i = 0; i < TOTAL; ++i) {
        int randNum = rand();
        mixPara[i] =    ((double )randNum) / ((double )RAND_MAX + 1);
    }

    QueryMeta* insertQueries = (QueryMeta*)malloc(sizeof(QueryMeta) * TOTAL );
    QueryMeta* queries = (QueryMeta*)malloc(sizeof(QueryMeta) * TOTAL );
    QueryMeta* removeQuery = (QueryMeta*)malloc(sizeof(QueryMeta) * TOTAL);

    start = clock();
    for(int i = 0; i < TOTAL ;i ++){
        QueryMetaConstructor(queries + i);
        QueryMetaConstructor(insertQueries + i);
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
    printLog = 1;
    pthread_t thread[MaxThread];
    ThreadAttributes attributes[MaxThread];
//    start = clock();
    for (int i = 0; i < threadnum; ++i) {
        attributes[i].start = i * perThread;
        attributes[i].end = i == (threadnum - 1)? TOTAL: (i + 1)* perThread;
        attributes[i].insertQueries = insertQueries;
        attributes[i].queries = queries;
        attributes[i].removeQuery = removeQuery;
        attributes[i].qTree = &qTree;
        attributes[i].mixPara = mixPara;
        pthread_create(&thread[i], 0, (void *(*)(void *))testInsert, (void *)&attributes[i]);
    }
    for (int i = 0; i < threadnum; ++i) {
        pthread_join(thread[i], NULL);
        putT += attributes[i].result.usedTime;
    }
    putT = putT / threadnum;
//    finish = clock();
//    putT = (double)(finish - start)/CLOCKS_PER_SEC;
    int num = qTree.elements;
    num += qTree.batchCount;
    printf("%d\n", num);
    if(NodeCheckLink(qTree.root) == FALSE){
        printf("NodeCheckLink ERROR!!!\n");
    }
    if(NodeCheckMaxMin(qTree.root) == FALSE){
        printf("NodeCheckMaxMin ERROR!!!\n");
    }
//    printQTree(&qTree);

    size_t removed = 0;
    QTreeResetStatistics(&qTree);
//    PAPI_init();
//    PAPI_startCache();
//    printLog = 1;
//    start = clock();

    for (int i = 0; i < threadnum; ++i) {
        attributes[i].start = i * perThread;
        attributes[i].end = i == (threadnum - 1)? TOTAL: (i + 1)* perThread;
        attributes[i].queries = queries;
        attributes[i].removeQuery = removeQuery;
        attributes[i].qTree = &qTree;
        attributes[i].mixPara = mixPara;
        pthread_create(&thread[i], 0, (void *(*)(void *))testMix, (void *)&attributes[i]);
    }

    for (int i = 0; i < threadnum; ++i) {
        pthread_join(thread[i], NULL);
        removed += attributes[i].result.size;
        mixT += attributes[i].result.usedTime;
    }
    mixT = mixT / threadnum;
//    finish = clock();
//    printQTree(&qTree);
//    PAPI_readCache();
//    PAPI_end();
    if(NodeCheckLink(qTree.root) == FALSE){
        printf("NodeCheckLink ERROR!!!\n");
    }
    if(NodeCheckMaxMin(qTree.root) == FALSE){
        printf("NodeCheckMaxMin ERROR!!!\n");
    }
    QTreeDestroy(&qTree);


//    mixT = (double)(finish - start)/CLOCKS_PER_SEC;
printf("%d, %d, %d,  %d, %d, %d, %.2lf, %d,  %d,  %.3lf,%.3lf,%.3lf, %d, %d, %ld, %ld, %ld,  %ld, %ld, %ld, %ld, %ld, %d, %d, %d, %d, %d\n",
       Border, checkQueryMeta, optimizationType, dataPointType, dataRegionTypeOld, searchKeyType, insertRatio, removePoint, TOTAL,
           generateT, putT, mixT, insertNum, removeNum, removed, checkQuery, checkLeaf, checkInternal,
           qTree.leafSplitCount, qTree.internalSplitCount, qTree.whileCount, qTree.funcCount, RemovedQueueSize, batchMissThreshold, MaxBatchCount, setKeyCount, threadnum);
    free(queries) ;
    free(removeQuery);
    free(insertQueries);
    return 0;
}

void testZipf(){
    int n = 41;
    int num[1000];
    initZipfParameter(n, zipfPara);
    for (int i = 0; i < n; ++i) {
        printf("%d, ", zipf());
        num[i] = zipf();
    }
    printf("\n");
    BFPRT(num, 0 , n -1, n / 2);
    for (int i = 0; i < n; ++i) {
        printf("%d, ", num[i] );
    }
    printf("\n");


}

int main(){
//    testZipf();
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