//
// Created by workshop on 9/1/2021.
//

#include <math.h>
#include <stdio.h>
#include "Tools.h"
#include "common.h"
#define N 999
/*
int zipf(double alpha, int n)
{
    static int first = 1;      // Static first time flag
    static double c = 0;          // Normalization constant
    static double *sum_probs;     // Pre-calculated sum of probabilities
    double z;                     // Uniform random number (0 < z < 1)
    int zipf_value;               // Computed exponential value to be returned
    int    i;                     // Loop counter
    int low, high, mid;           // Binary-search bounds

    // Compute normalization constant on first call only
    if (first == 1)
    {
        for (i=1; i<=n; i++)
            c = c + (1.0 / pow((double) i, alpha));
        c = 1.0 / c;

        sum_probs = (double *)(malloc((n + 1) * sizeof(*sum_probs)));
        sum_probs[0] = 0;
        for (i=1; i<=n; i++) {
            sum_probs[i] = sum_probs[i-1] + c / pow((double) i, alpha);
        }
        first = 0;
    }

    // Pull a uniform random number (0 < z < 1)
    do
    {
        z = (rand() % (N + 1) )/ (double)(N + 1);
    }
    while ((z == 0) || (z == 1));

    // Map z to the value
    low = 1, high = n, mid;
    do {
        mid = floor((low+high)/2);
        if (sum_probs[mid] >= z && sum_probs[mid-1] < z) {
            zipf_value = mid;
            break;
        } else if (sum_probs[mid] >= z) {
            high = mid-1;
        } else {
            low = mid+1;
        }
    } while (low <= high);


    return(zipf_value);
}
 */

/**
   * Number of items.
   */
 long items;

/**
 * Min item to generate.
 */
 long base;

/**
 * The zipfian constant to use.
 */
double zipfianconstant;

/**
 * Computed parameters for generating the distribution.
 */
 double alpha, zetan, eta, theta, zeta2theta;

/**
 * The number of items used to compute zetan the last time.
 */
 long countforzeta;

 void initZipfParameter(int n, double zipfianpara){
     items = n;
     base = 0;
     zipfianconstant = zipfianpara;
     theta = zipfianconstant;
     zeta2theta = zeta(2, theta);
     alpha = 1.0 / (1.0 - theta);
     zetan = zetastatic(n, zipfianpara);
     countforzeta = items;
     eta = (1 - pow(2.0 / items, 1 - theta)) / (1 - zeta2theta / zetan);
 }

/**
 * Compute the zeta constant needed for the distribution. Do this from scratch for a distribution with n items,
 * using the zipfian constant theta. This is a static version of the function which will not remember n.
 * @param n The number of items to compute zeta over.
 * @param theta The zipfian constant.
 */
 double zetastatic(long n, double theta) {
    double sum = 0;
    for (long i = 0; i < n; i++) {

        sum += 1 / (pow(i + 1, theta));
    }

    return sum;
}
/**
 * Compute the zeta constant needed for the distribution. Do this from scratch for a distribution with n items,
 * using the zipfian constant thetaVal. Remember the value of n, so if we change the itemcount, we can recompute zeta.
 *
 * @param n The number of items to compute zeta over.
 * @param thetaVal The zipfian constant.
 */
double zeta(long n, double thetaVal) {
    return zetastatic(n, thetaVal);
}




// from ycsb
int zipf(){
    double u = ((double )rand()) / ((double )RAND_MAX + 1);
    double uz = u * zetan;

    if (uz < 1.0) {
        return base;
    }

    if (uz < 1.0 + pow(0.5, theta)) {
        return base + 1;
    }

    int ret =  (int) ((items) * pow(eta * u - eta + 1, alpha));
    return ret;
}


void printArray(int* array, int num){

    for(int i = 0; i < num; i ++){
        printf("%d,",  *(array + i));
    }
    printf("\n");
}

void bubbleSort(void* arr[], int len, BOOL (*cmp)(void*, void *)) {
    int i, j;
    void *temp;
    for (i = 0; i < len - 1; i++)
        for (j = 0; j < len - 1 - i; j++)
            if (cmp(arr[j], arr[j + 1])) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
}

char *myItoa(int num, char *str){
    if(str == NULL)
    {
        return NULL;
    }
    sprintf(str, "%d", num);
    return str;
}