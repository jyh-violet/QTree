//
// Created by workshop on 9/5/2021.
//
#include "QTree.h"

inline void LeafNodeConstructor(LeafNode* leafNode, QTree *tree){
    memset(leafNode, 0, sizeof (LeafNode));
    NodeConstructor((Node*)leafNode, tree);
//    leafNode->values = malloc(sizeof (ValueType *) * tree->Border);
//    memset(leafNode->values,0, sizeof (ValueType *) * Border);
}
void LeafNodeDestroy(LeafNode* leafNode){
    free(leafNode);
//    free(leafNode->values);
}

void LeafNodeMerge(LeafNode* leafNode, InternalNode* nodeParent, int slot,
                   Node* nodeFROMx){
    LeafNode* nodeFROM = (LeafNode*) nodeFROMx;
    LeafNode* nodeTO = leafNode;
    int sizeTO = nodeTO->node.allocated;
    int sizeFROM = nodeFROM->node.allocated;
    if(sizeFROM > 0){
        // copy keys from nodeFROM to nodeTO
        memcpy(nodeTO->node.keys + sizeTO , nodeFROM->node.keys, sizeFROM * sizeof(KeyType ));
        memcpy(nodeTO->values + sizeTO , nodeFROM->values, (sizeFROM) * sizeof(ValueType *));

        nodeTO->node.allocated += sizeFROM; // keys of FROM and key of nodeParent
        if(sizeTO == 0 || (sizeFROM > 0 && nodeTO->node.maxValue < nodeFROM->node.maxValue)){
            nodeTO->node.maxValue = nodeFROM->node.maxValue;
        }
        if(sizeTO == 0 || (sizeFROM > 0 && (nodeTO->node.minValue) > (nodeFROM->node.minValue))){
            nodeTO->node.minValue = nodeFROM->node.minValue;
        }
    }


    // remove key from nodeParent
    InternalNodeRemove(nodeParent, slot);
    // Free nodeFROM
    free((Node*)nodeFROM);
}

void * LeafNodRemove(LeafNode* leafNode, int slot) {
    return NULL;
}

BOOL LeafNodeAdd(LeafNode* leafNode, int slot, KeyType * newKey, ValueType * newValue){
    if (slot < leafNode->node.allocated) {
        memcpy(leafNode->node.keys + slot + 1, leafNode->node.keys + slot, (leafNode->node.allocated - slot) * sizeof(KeyType ));
        memcpy(leafNode->values + slot + 1, leafNode->values + slot, (leafNode->node.allocated - slot) * sizeof(ValueType *));
    }
    leafNode->node.allocated++;
    leafNode->node.keys[slot] = *newKey;
    leafNode->values[slot] = newValue;
    if(leafNode->node.allocated == 1 || (newKey->upper >  (leafNode->node.maxValue))){
        leafNode->node.maxValue = newKey->upper;
    }
    if(leafNode->node.allocated == 1 || ((leafNode->node.minValue) >  newKey->lower)){
        leafNode->node.minValue = newKey->lower;
    }
    return TRUE;
}
BOOL LeafNodeAddBatch(LeafNode* leafNode, int slot, QueryData batch[], int batchCount, BoundKey *min, BoundKey* max){
    if(leafNode->node.allocated + batchCount > Border){
        printf("LeafNodeAddBatch, batch too large. allocated:%d, batchCount:%d\n", leafNode->node.allocated, batchCount);
    }
    if (slot < leafNode->node.allocated) {
        memcpy(leafNode->node.keys + slot + batchCount, leafNode->node.keys + slot, (leafNode->node.allocated - slot) * sizeof(KeyType ));
        memcpy(leafNode->values + slot + batchCount, leafNode->values + slot, (leafNode->node.allocated - slot) * sizeof(ValueType *));
    }

    for (int i = 0; i < batchCount; ++i) {
        leafNode->node.keys[slot + i] = batch[i].key;
        leafNode->values[slot + i] = batch[i].value;
        if( (batch[i].key.upper >  *max)){
            *max = batch[i].key.upper;
        }
        if( (batch[i].key.lower <  *min)){
            *min = batch[i].key.lower;
        }
    }
    if(leafNode->node.allocated == 0 || leafNode->node.maxValue < *max){
        leafNode->node.maxValue = *max;
    }
    if(leafNode->node.allocated == 0 || leafNode->node.minValue > *min){
        leafNode->node.minValue = *min;
    }
    leafNode->node.allocated += batchCount;
    return TRUE;
}

BOOL LeafNodeAddLast(LeafNode* leafNode, KeyType * newKey, ValueType * newValue){
    leafNode->node.keys[leafNode->node.allocated] = *newKey;
    leafNode->values[leafNode->node.allocated] = newValue;
    leafNode->node.allocated++;
    if(leafNode->node.allocated == 1 || (newKey->upper >  (leafNode->node.maxValue))){
        leafNode->node.maxValue = newKey->upper;
    }
    if(leafNode->node.allocated == 1 || ((leafNode->node.minValue) >  newKey->lower)){
        leafNode->node.minValue = newKey->lower;
    }
    return TRUE;
}

inline void LeafNodeAllocId(LeafNode* leafNode) {
    leafNode->node.id = QTreeAllocNode(leafNode->node.tree, (TRUE));
}

inline void LeafNodeResetMaxValue(LeafNode* leafNode) {
    if (leafNode->node.allocated == 0) {
        return;
    }
    leafNode->node.maxValue = leafNode->node.keys[0].upper;
    for (int i = 1; i < leafNode->node.allocated; i++) {
        if ((leafNode->node.keys[i].upper) > (leafNode->node.maxValue)) {
            leafNode->node.maxValue = leafNode->node.keys[i].upper;
        }
    }
}


inline void LeafNodeResetMinValue(LeafNode* leafNode) {
    if (leafNode->node.allocated == 0) {
        return;
    }
    leafNode->node.minValue = leafNode->node.keys[0].lower;
    for (int i = 1; i < leafNode->node.allocated; i++) {
        if (((leafNode->node.minValue)> leafNode->node.keys[i].lower)) {
            leafNode->node.minValue = leafNode->node.keys[i].lower;
        }
    }
}

inline void swap(KeyType arr[], int a, int b){
    KeyType tmp = arr[a];
    arr[a] = arr[b];
    arr[b] = tmp;
}

void InsertSortIndex(KeyType data[], short index[],  int l, int r)
{
    for(int i = l + 1; i <= r; i++)
    {
        if(data[index[i - 1]].searchKey > data[index[i]].searchKey)
        {
            int t = index[i];
            int j = i;
            while(j > l && data[index[j - 1]].searchKey > data[t].searchKey)
            {
                index[j] = index[j - 1];
                j--;
            }
            index[j] = t;
        }
    }
}

int FindMidIndex(KeyType data[], short index[], int l, int r)
{
    if(l == r) return l;
    int i = 0;
    int n = 0;
    for(i = l; i < r - 5; i += 5)
    {
        InsertSortIndex(data, index, i, i + 4);
        n = i - l;
        short temp = index[l + n / 5];
        index[l + n / 5] = index[i + 2];
        index[i + 2] = temp;
    }

    //处理剩余元素
    int num = r - i + 1;
    if(num > 0)
    {
        InsertSortIndex(data, index, i, i + num - 1);
        n = i - l;
        short temp = index[l + n / 5];
        index[l + n / 5] = index[i + num / 2];
        index[i + num / 2] = temp;
    }
    n /= 5;
    if(n == l) return l;
    return FindMidIndex(data, index, l, l + n);
}

int PartionIndex(KeyType data[], short index[], int l, int r, int p)
{
    short temp = index[p];
    index[p] = index[l];
    index[l] = temp;
    int i = l;
    int j = r;
    short pivot = index[l];
    while(i < j)
    {
        while(data[index[j]].searchKey >= data[pivot].searchKey && i < j)
            j--;
        index[i] = index[j];
        while(data[index[i]].searchKey <= data[pivot].searchKey && i < j)
            i++;
        index[j] = index[i];
    }
    index[i] = pivot;
    return i;
}

int BFPRTSelect(KeyType data[], short index[], int l, int r, int k)
{
    int p = FindMidIndex(data, index, l, r);    //寻找中位数的中位数
    int i = PartionIndex(data, index, l, r, p);

    int m = i - l + 1;
    if(m == k) return index[i];
    if(m > k)  return BFPRTSelect(data, index, l, i - 1, k);
    return BFPRTSelect(data, index, i + 1, r, k - m);
}

inline void quickSelect(KeyType data[], short index[], int k, int s, int e){
    if(s > e || k < s || k > e){
        printf("invalid array range\n");
        return;
    }

    short i, j;
    short pivot = index[s];
    if(s <= e){
        i = s;
        j = e;
        for(;;){
            while(data[index[j]].searchKey >= data[pivot].searchKey && i<j){j--;}
            while(data[index[i]].searchKey <= data[pivot].searchKey && i<j){i++;}
            if(i<j){
                short temp = index[i];
                index[i] = index[j];
                index[j] = temp;
            }
            else
                break;
        }
        short temp = index[i];
        index[i] = index[s];
        index[s] = temp;
        if(k<i)
            quickSelect(data, index, k, s, i-1);
        else if(k>i)
            quickSelect(data, index, k, i+1, e);
    }
}

Node* LeafNodeSplit_Sort(LeafNode* leafNode) {
    if(leafNode->node.tree == NULL){
        printLeafNode(leafNode);
    }
    leafNode->node.tree->leafSplitCount ++;
    LeafNode* newHigh = (LeafNode*)malloc(sizeof (LeafNode));
    LeafNodeConstructor(newHigh, leafNode->node.tree);
    LeafNodeAllocId(newHigh);

    int j = leafNode->node.allocated >> 1; // dividir por dos (libro)
    int newsize = leafNode->node.allocated - j;
    // if (log.isDebugEnabled()) log.debug("split j=" + j);
    memcpy(newHigh->node.keys, leafNode->node.keys + j,  newsize * sizeof (KeyType ));

    memcpy(newHigh->values, leafNode->values + j,  newsize * sizeof (ValueType *));

    memset(leafNode->node.keys + j, 0, sizeof (KeyType) * newsize);
    memset(leafNode->values + j, 0, sizeof (ValueType*) * newsize);
    //    for (int i = j; i < j + newsize; i++) {
    //        leafNode->node.keys[i] = NULL;
    //        // clear bound id of the influenced query
    //        leafNode->values[i] = NULL;
    //    }
    newHigh->node.allocated = newsize;
    leafNode->node.allocated -= newsize;
    LeafNodeResetMaxValue(leafNode);
    LeafNodeResetMaxValue(newHigh);
    LeafNodeResetMinValue(leafNode);
    LeafNodeResetMinValue(newHigh);
    return (Node*)newHigh;
}


Node* LeafNodeSplit_NoSort(LeafNode* leafNode) {
    if(leafNode->node.tree == NULL){
        printLeafNode(leafNode);
    }
    leafNode->node.tree->leafSplitCount ++;
    LeafNode* newHigh = (LeafNode*)malloc(sizeof (LeafNode));
    LeafNodeConstructor(newHigh, leafNode->node.tree);
    LeafNodeAllocId(newHigh);
    int median = leafNode->node.allocated >> 1;

    if(useBFPRT){
        short index[Border];
        for (int i = 0; i < leafNode->node.allocated; ++i) {
            index[i] = i;
        }
        BFPRTSelect(leafNode->node.keys,  index, 0, leafNode->node.allocated - 1, median);
        int oldSize = leafNode->node.allocated;
        char flags[Border];
        memset(flags, 0, Border);
        for (int i = median; i < oldSize; ++i) {
            flags[index[i]] = 1;
        }
        leafNode->node.allocated = 0;
        for (int i = 0; i < oldSize; ++i) {
            if(flags[i]){
                LeafNodeAddLast(newHigh, leafNode->node.keys + i, leafNode->values[i]);
            } else{
                LeafNodeAddLast(leafNode, leafNode->node.keys + i, leafNode->values[i]);
            }

        }
    } else{
//        KeyType copyKey[Border];
//        memcpy(copyKey, leafNode->node.keys, leafNode->node.allocated * sizeof (KeyType));
        short index[Border];
        for (int i = 0; i < leafNode->node.allocated; ++i) {
            index[i] = i;
        }
        quickSelect(leafNode->node.keys, index, median, 0, leafNode->node.allocated - 1);
        int oldSize = leafNode->node.allocated;
        char flags[Border];
        memset(flags, 0, Border);
        for (int i = median; i < oldSize; ++i) {
            flags[index[i]] = 1;
        }
        leafNode->node.allocated = 0;
        for (int i = 0; i < oldSize; ++i) {
            if(flags[i]){
                LeafNodeAddLast(newHigh, leafNode->node.keys + i, leafNode->values[i]);
            } else{
                LeafNodeAddLast(leafNode, leafNode->node.keys + i, leafNode->values[i]);
            }

        }
    }



    return (Node*)newHigh;
}

Node*  LeafNodeSplit(LeafNode* leafNode) {
    switch (optimizationType) {
        case None:
        case Batch:
            return LeafNodeSplit_Sort(leafNode);
        case NoSort:
        case BatchAndNoSort:
            return LeafNodeSplit_NoSort(leafNode);
        default:
            printf("LeafNodeSplit unsupported optimizationType:%d\n", optimizationType);
            return NULL;

    }
}

inline void LeafNodeResetMinKey(LeafNode* leafNode){
    int minIndex = 0;
    for(int i = 1; i < leafNode->node.allocated; i ++){
        if(QueryRangeLT(leafNode->node.keys[i], leafNode->node.keys[minIndex])){
            minIndex = i;
        }
    }
    swap(leafNode->node.keys, 0, minIndex);
    ValueType* temp = leafNode->values[0];
    leafNode->values[0] = leafNode->values[minIndex];
    leafNode->values[minIndex] = temp;
}

KeyType  LeafNodeSplitShiftKeysLeft(LeafNode* leafNode) {
    if(optimizationType == NoSort || optimizationType == BatchAndNoSort){
        LeafNodeResetMinKey(leafNode);
    }
    return leafNode->node.keys[0];
}

int LeafNodeGetId(LeafNode* leafNode) {
    return leafNode->node.id;
}

int LeafNodeGetHeight(LeafNode* leafNode) {
    return 1;
}

void LeafNodeResetId(LeafNode* leafNode){
    for (int i = 0; i < leafNode->node.allocated ; ++i) {
        myItoa(Qid, getQueryId(leafNode->values[i]));
        Qid ++;
    }
}


void printLeafNode(LeafNode* leafNode){
    printf("[L%d](%d)(%d,%d){", leafNode->node.id, leafNode->node.allocated,
           ((leafNode->node.minValue)), ((leafNode->node.maxValue)));
    for (int i = 0; i < leafNode->node.allocated; i++) {
        QueryRange * k = &leafNode->node.keys[i];
        QueryMeta* v = leafNode->values[i];
//        printQueryRange(k);
        printf("%d:", k->searchKey);
        printQueryRange(k);
        printf("Q[%s] | ",v->queryId );
    }
    printf("}\n");
}

BOOL LeafNodeCheckMaxMin(LeafNode * leafNode){
    if(leafNode->node.allocated == 0){
        return TRUE;
    }
    int findMin = FALSE, findMax = FALSE;
    for(int i = 0; i < leafNode->node.allocated; i ++){
        if(leafNode->node.keys[i].lower == leafNode->node.minValue){
            findMin = TRUE;
        }
        if(leafNode->node.keys[i].upper == leafNode->node.maxValue){
            findMax = TRUE;
        }
    }
    if(findMin == TRUE && findMax ==TRUE){
        return TRUE;
    } else{
        return FALSE;
    }
}

BOOL LeafNodeCheckKey(LeafNode * leafNode){
    for (int i = 0; i < leafNode->node.allocated; ++i) {
        if(leafNode->node.keys[i].upper != leafNode->values[i]->dataRegion.upper || leafNode->node.keys[i].lower != leafNode->values[i]->dataRegion.lower){
            return FALSE;
        }
    }
    return TRUE;
}