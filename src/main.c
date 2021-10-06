#include "query/QueryRange.h"
#include "query/QueryMeta.h"
#include "index/QTree.h"
#include <libconfig.h>
#include <time.h>
#include <Tool/ArrayList.h>


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

int test() {
#undef BOrder_65
#define BOrder_129
    double generateT = 0, putT = 0, removeT = 0, mixT = 0;
//    TOTAL = 1000;
    TRACE_LEN = 100000;
    srand((unsigned)time(NULL));
    clock_t   start,   finish, time1, time2;
    QTree qTree;
    QTreeConstructor(&qTree, 2);
    double *mixPara = (double *) malloc(sizeof (double ) * TOTAL);
    for (int i = 0; i < TOTAL; ++i) {
        int randNum = rand();
        mixPara[i] =    ((double )randNum) / ((double )RAND_MAX + 1);
    }

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

    time1 = start = clock();
//    for(    int i = 0; i < TOTAL; i ++){
//        QTreePut(&qTree, &(queries[i].dataRegion), queries + i);
//    }
//    finish = clock();
//    putT = (double)(finish - start)/CLOCKS_PER_SEC;
//    int num = qTree.elements;
//    num += qTree.batchCount;
//    printf("%d\n", num);
//    printQTree(&qTree);


    Arraylist* removedQuery = ArraylistCreate(TOTAL);
    time1 = start = clock();
    int insertNum = 0, removeNum = 0;
    for (int i = 0; i < TOTAL; ++i) {

        if(mixPara[i] < insertRatio){
            QTreePut(&qTree, &(queries[i].dataRegion), queries + i);
            insertNum ++;
        } else{
            QTreeFindAndRemoveRelatedQueries(&qTree, (removeQuery[i].dataRegion.upper + removeQuery[i].dataRegion.lower) / 2, removedQuery);
            removeNum ++;
        }
//        if(QTreeCheckKey(&qTree) ==FALSE){
//            printQTree(&qTree);
//            printf("QTreeCheckKey error: %d, %d\n", i, mixPara[i] < insertRatio? 0 : 1);
////            exit(-1);
//        }
//        if(QTreeCheckMaxMin(&qTree) ==FALSE){
//            printQTree(&qTree);
//            printf("QTreeCheckMaxMin error: %d, %d\n", i, mixPara[i] < insertRatio? 0 : 1);
//            //            exit(-1);
//        }
//        if((i + 1) % TRACE_LEN == 0){
//            time2 = clock();
//            printf("%d, used %lf s \n", i, (double)(time2 - time1)/CLOCKS_PER_SEC);
//            time1 = time2;
//        }
    }
    finish = clock();
    size_t removed = removedQuery->size;
//    printf( "get and remove end!\n remain:%d\n",  qTree.elements);
    ArraylistDeallocate(removedQuery);
//    printQTree(&qTree);
    QTreeDestroy(&qTree);

    mixT = (double)(finish - start)/CLOCKS_PER_SEC;
//    printf("remove end! use %lfs\n", (double)(finish - start)/CLOCKS_PER_SEC);
//    printf( "get and remove end!\n remain:%d\n",  qTree.elements);
printf("%d, %d, %d,  %d, %d, %d, %.2lf, %d,  %d,  %.3lf,%.3lf,%.3lf, %d, %d, %ld, %ld, %ld,  %ld, %ld, %ld, %ld, %ld, %d, %d, %d\n",
       Border, checkQueryMeta, optimizationType, dataPointType, dataRegionTypeOld, searchKeyType, insertRatio, removePoint, TOTAL,
           generateT, putT, mixT, insertNum, removeNum, removed, checkQuery, checkLeaf, checkInternal,
           qTree.leafSplitCount, qTree.internalSplitCount, qTree.whileCount, qTree.funcCount, RemovedQueueSize, batchMissThreshold, MaxBatchCount);
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

    maxValue = TOTAL;

    test();
    return 0;
}