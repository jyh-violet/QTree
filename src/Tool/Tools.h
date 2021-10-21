//
// Created by jyh_2 on 2021/5/14.
//

#ifndef QTREE_TOOLS_H
#define QTREE_TOOLS_H
#define MAX_LOG_SIZE 1000
#define LOG_PATH "log"

typedef enum LOGLevel{
    InsertLog,
    MiXLog
}LOGLevel;

typedef enum DataRegionType {
    Same,
    Random,
    Increase,
    Zipf,
    Remove
}DataRegionType;

typedef enum DataPointType{
    RemovePoint,
    LowPoint,
    MidPoint
}DataPointType;

typedef enum SearchKeyType{
    LOW,
    DYMID,
    Mid,
    REMOVE,
    RAND
}SearchKeyType;

typedef enum OptimizationType{
    None,
    NoSort,
    BatchAndNoSort,
    Batch
}OptimizationType;

typedef enum BOOL{
    FALSE,
    TRUE
}BOOL;


void handle_error (int retval);
void PAPI_init();
void PAPI_startCache();
void PAPI_readCache();
void PAPI_end();

void initZipfParameter(int n, double zipfianpara);
int zipf();
double zeta(long n, double thetaVal) ;
double zetastatic(long n, double theta) ;

void printArray(int* array, int num);
void bubbleSort(void* arr[], int len, BOOL (*cmp)(void*, void *));
char *myItoa(int num, char *str);


int BFPRT(int a[], int l, int r, int k);
int Partion(int a[], int l, int r, int p);
int FindMid(int a[], int l, int r);
void InsertSort(int a[], int l, int r);

void vmlog(LOGLevel logLevel, char* fmat, ...);

#endif //QTREE_TOOLS_H
