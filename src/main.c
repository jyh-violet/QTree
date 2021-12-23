#include "query/QueryRange.h"
#include "query/QueryMeta.h"
#include "index/QTree.h"
#include <libconfig.h>
#include <time.h>
#include <Tool/ArrayList.h>
#include <papi.h>
#include <pthread.h>

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
double deleteRatio = 0;

u_int64_t checkLeaf = 0;
u_int64_t checkQuery = 0;
u_int64_t checkInternal = 0;
int removePoint = 0;
double zipfPara = 0.75;
int rangeWidth = 100;
BOOL qtreeCheck = FALSE;

_Atomic int insertNum = 0, removeNum = 0, deleteNum = 0;
int threadnum = 4;
//pthread_key_t threadId;
typedef struct TaskRes{
    double usedTime;
    size_t size;
}TaskRes;
typedef struct ThreadAttributes{
    int threadId;
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
//    pthread_setspecific(threadId, (void *)attributes->threadId);
struct timespec startTmp, endTmp;
clock_gettime(CLOCK_REALTIME, &startTmp);
    for(    int i = attributes->start; i <  attributes->end; i ++){
        int index = (i - attributes->start) * threadnum + attributes->threadId;
        vmlog(RemoveLog,"i:%d, key:%d",i, attributes->insertQueries[index].dataRegion.lower);
        QTreePut(attributes->qTree, attributes->insertQueries + index, attributes->threadId);
//        if((i + 1) % 100000 == 0){
//            vmlog(InsertLog,"insert:%d", i);
//        }
    }
    clock_gettime(CLOCK_REALTIME, &endTmp);
    attributes->result.usedTime = (endTmp.tv_sec - startTmp.tv_sec) + (endTmp.tv_nsec - startTmp.tv_nsec) * 1e-9;
    attributes->result.size = attributes->end - attributes->start;
}

void testMix(ThreadAttributes* attributes){
//    pthread_setspecific(threadId, (void *)attributes->threadId);
    Arraylist* removedQuery = ArraylistCreate(attributes->end - attributes->start);
    struct timespec startTmp, endTmp;
    clock_gettime(CLOCK_REALTIME, &startTmp);
    for (int i = attributes->start; i <  attributes->end; ++i) {
//        vmlog(RemoveLog,"thread:%d, i:%d, para:%lf, rm:%ld", attributes->threadId, i, attributes->mixPara[i], removedQuery->size);
        if(attributes->mixPara[i] < insertRatio){
            QTreePut(attributes->qTree, attributes->queries + i, attributes->threadId);
            insertNum ++;
        }else if(attributes->mixPara[i] < (insertRatio + deleteRatio)){
            if(markDelete){
                if(QTreeMarkDelete(attributes->qTree, attributes->insertQueries + i) == TRUE){
                    attributes->result.size ++;
                }
            } else{
                if (QTreeDeleteQuery(attributes->qTree, attributes->insertQueries + i, attributes->threadId) == TRUE){
                    attributes->result.size ++;
                }
            }

            deleteNum ++;
        } else{
            QTreeFindAndRemoveRelatedQueries(attributes->qTree,
                                             (attributes->removeQuery[i].dataRegion.upper + attributes->removeQuery[i].dataRegion.lower) / 2,
                                             removedQuery,
                                             attributes->threadId);

            removeNum ++;
        }
    }
//    printf("thread:%ld, delete:%d\n", attributes->threadId, attributes->result.size);
    clock_gettime(CLOCK_REALTIME, &endTmp);
    attributes->result.usedTime = (endTmp.tv_sec - startTmp.tv_sec) + (endTmp.tv_nsec - startTmp.tv_nsec) * 1e-9;
    attributes->result.size += removedQuery->size;
    ArraylistDeallocate(removedQuery);
}


int test() {
//    markDelete = FALSE;
//    threadnum = 4;
    useBFPRT = 0;
    double generateT = 0, putT = 0,  mixT = 0;
//    TOTAL = 10000000;
//    dataRegionType = Increase;
//    maxValue = TOTAL / 10;
    maxValue = 1 << valueSpan;
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
//    dataRegionType = Remove;
    for(int i = 0; i < TOTAL;i ++){
        QueryMetaConstructor(removeQuery + i);
    }
    finish = clock();
    generateT = (double)(finish - start)/CLOCKS_PER_SEC;
//    printf("generate end! use %lfs\n", (double)(finish - start)/CLOCKS_PER_SEC );
//    printLog = 1;
    int perThread = TOTAL / threadnum;
    pthread_t thread[MaxThread];
    ThreadAttributes attributes[MaxThread];
    memset(attributes, 0, sizeof (ThreadAttributes) * MaxThread);
//    start = clock();
//    pthread_key_create(&threadId, key_destrutor);
    for (int i = 0; i < threadnum; ++i) {
        attributes[i].threadId = i;
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
    for (int i = 0; i < threadnum; ++i) {
        num += qTree.batchCount[i];
    }
//
//    if(NodeCheckLink(qTree.root) == FALSE){
//        printf("NodeCheckLink ERROR!!!\n");
//    }
//    if(NodeCheckMaxMin(qTree.root) == FALSE){
//        printf("NodeCheckMaxMin ERROR!!!\n");
//    }
//    printf("%d, %d, %d\n", num, NodeGetHeight(qTree.root), qTree.height);

//    printQTree(&qTree);

    size_t removed = 0;
    QTreeResetStatistics(&qTree);
//    PAPI_init();
//    PAPI_startCache();
    printLog = 1;
//    start = clock();
//    TOTAL = 100;
    perThread = TOTAL / threadnum;
    memset(attributes, 0, sizeof (ThreadAttributes) * MaxThread);
    for (int i = 0; i < threadnum; ++i) {
        attributes[i].threadId = i;
        attributes[i].start = i * perThread;
        attributes[i].end = i == (threadnum - 1)? TOTAL: (i + 1)* perThread;
        attributes[i].insertQueries = insertQueries;
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
//    if(NodeCheckLink(qTree.root) == FALSE){
//        printf("NodeCheckLink ERROR!!!\n");
//    }
//    if(NodeCheckMaxMin(qTree.root) == FALSE){
//        printf("NodeCheckMaxMin ERROR!!!\n");
//    }
//    num = qTree.elements;
//    for (int i = 0; i < threadnum; ++i) {
//        num += qTree.batchCount[i];
//    }
//    printf("%d, %d, %d\n", num, NodeGetHeight(qTree.root), qTree.height);

    if(markDelete){
        WorkEnd = TRUE;
        while (RefactorThreadEnd == FALSE){
            WorkEnd = TRUE;
        }
    }
    QTreeDestroy(&qTree);

//    mixT = (double)(finish - start)/CLOCKS_PER_SEC;
printf("%d, %d, %d,  %d, %d, %d, %.2lf, %.2lf, %d,  %d,  %.3lf,%.3lf,%.3lf, %d, %d, %d, %ld, %ld, %ld,  %ld, %ld, %ld, %ld, %ld, %d, %d, %d, %d, %.2lf, %d, %d, %d, %d\n",
       Border, checkQueryMeta, optimizationType, dataPointType, dataRegionTypeOld, searchKeyType, insertRatio, deleteRatio, removePoint, TOTAL,
           generateT, putT, mixT, insertNum, deleteNum, removeNum, removed, checkQuery, checkLeaf, checkInternal,
           qTree.leafSplitCount, qTree.internalSplitCount, qTree.whileCount, qTree.funcCount, RemovedQueueSize, batchMissThreshold, MaxBatchCount,
           setKeyCount, zipfPara, rangeWidth, threadnum, markDelete, valueSpan);
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

void simpleTest(){
    srand((unsigned)time(NULL));
    QTree qTree;
    QTreeConstructor(&qTree, 2);
    TOTAL = 100;
    dataRegionType = Random;
    double *mixPara = (double *) malloc(sizeof (double ) * TOTAL);
    for (int i = 0; i < TOTAL; ++i) {
        int randNum = rand();
        mixPara[i] =    ((double )randNum) / ((double )RAND_MAX + 1);
    }

    QueryMeta* insertQueries = (QueryMeta*)malloc(sizeof(QueryMeta) * TOTAL );
    QueryMeta* queries = (QueryMeta*)malloc(sizeof(QueryMeta) * TOTAL );
    QueryMeta* removeQuery = (QueryMeta*)malloc(sizeof(QueryMeta) * TOTAL);

    for(int i = 0; i < TOTAL ;i ++){
        QueryMetaConstructor(queries + i);
        QueryMetaConstructor(insertQueries + i);
        QueryMetaConstructor(removeQuery + i);
    }
    for(int i = 0; i < TOTAL ;i ++){
        QTreePut(&qTree, insertQueries + i, 0);
    }
    printQTree(&qTree);
    Arraylist* removedQuery = ArraylistCreate(TOTAL);
    for(int i = 0; i < TOTAL / 2 ;i ++){
        QTreeFindAndRemoveRelatedQueries(&qTree,
                                         (insertQueries[i].dataRegion.upper + insertQueries[i].dataRegion.lower) / 2,
                                         removedQuery,
                                         0);
    }
    printQTree(&qTree);

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
    config_lookup_float(&cfg, "deleteRatio", &deleteRatio);
    config_lookup_int(&cfg, "threadnum", &threadnum);
    config_lookup_float(&cfg, "zipfPara", &zipfPara);
    config_lookup_int(&cfg, "rangeWidth", &rangeWidth);
    config_lookup_int(&cfg, "markDelete", &markDelete);
    maxValue = TOTAL;

    test();
//    simpleTest();
    return 0;
}