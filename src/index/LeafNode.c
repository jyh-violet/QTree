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
        memcpy(nodeTO->data + sizeTO , nodeFROM->data, sizeFROM * sizeof(QueryData ));

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
        memcpy(leafNode->data + slot + 1, leafNode->data + slot, (leafNode->node.allocated - slot) * sizeof(QueryData ));
    }
    leafNode->node.allocated++;
    leafNode->data[slot].key = *newKey;
    leafNode->data[slot].value = newValue;
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
        memcpy(leafNode->data + slot + batchCount, leafNode->data + slot, (leafNode->node.allocated - slot) * sizeof(QueryData ));
    }
    memcpy(leafNode->data + slot, batch, batchCount * sizeof (QueryData));

    for (int i = 0; i < batchCount; ++i) {
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
    leafNode->data[leafNode->node.allocated].key = *newKey;
    leafNode->data[leafNode->node.allocated].value = newValue;
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
    leafNode->node.maxValue = leafNode->data[0].key.upper;
    for (int i = 1; i < leafNode->node.allocated; i++) {
        if ((leafNode->data[i].key.upper) > (leafNode->node.maxValue)) {
            leafNode->node.maxValue = leafNode->data[i].key.upper;
        }
    }
}


inline void LeafNodeResetMinValue(LeafNode* leafNode) {
    if (leafNode->node.allocated == 0) {
        return;
    }
    leafNode->node.minValue = leafNode->data[0].key.lower;
    for (int i = 1; i < leafNode->node.allocated; i++) {
        if (((leafNode->node.minValue)> leafNode->data[i].key.lower)) {
            leafNode->node.minValue = leafNode->data[i].key.lower;
        }
    }
}

inline void swap(KeyType arr[], int a, int b){
    KeyType tmp = arr[a];
    arr[a] = arr[b];
    arr[b] = tmp;
}

void InsertSortIndex(QueryData data[], int l, int r)
{
    for(int i = l + 1; i <= r; i++)
    {
        if(data[i - 1].key.searchKey > data[i].key.searchKey)
        {
            QueryData t = data[i];
            int j = i;
            while(j > l && data[j - 1].key.searchKey > t.key.searchKey)
            {
                data[j] = data[j - 1];
                j--;
            }
            data[j] = t;
        }
    }
}

int FindMidIndex(QueryData data[], int l, int r)
{
    if(l == r) return l;
    int i = 0;
    int n = 0;
    for(i = l; i < r - 5; i += 5)
    {
        InsertSortIndex(data, i, i + 4);
        n = i - l;
        QueryData temp = data[l + n / 5];
        data[l + n / 5] = data[i + 2];
        data[i + 2] = temp;
    }

    //处理剩余元素
    int num = r - i + 1;
    if(num > 0)
    {
        InsertSortIndex(data, i, i + num - 1);
        n = i - l;
        QueryData temp = data[l + n / 5];
        data[l + n / 5] = data[i + num / 2];
        data[i + num / 2] = temp;
    }
    n /= 5;
    if(n == l) return l;
    return FindMidIndex(data, l, l + n);
}

int PartionIndex(QueryData data[], int l, int r, int p)
{
    QueryData temp = data[p];
    data[p] = data[l];
    data[l] = temp;
    int i = l;
    int j = r;
    QueryData pivot = data[l];
    while(i < j)
    {
        while(data[j].key.searchKey >= pivot.key.searchKey && i < j)
            j--;
        data[i] = data[j];
        while(data[i].key.searchKey <= pivot.key.searchKey && i < j)
            i++;
        data[j] = data[i];
    }
    data[i] = pivot;
    return i;
}

int BFPRTSelect(QueryData data[], int l, int r, int k)
{
    int p = FindMidIndex(data,  l, r);    //寻找中位数的中位数
    int i = PartionIndex(data,  l, r, p);

    int m = i - l + 1;
    if(m == k) return i;
    if(m > k)  return BFPRTSelect(data, l, i - 1, k);
    return BFPRTSelect(data, i + 1, r, k - m);
}

inline void quickSelect(QueryData data[], int k, int s, int e){
    if(s > e || k < s || k > e){
        printf("invalid array range\n");
        return;
    }

    short i, j;
    QueryData pivot = data[s];
    if(s <= e){
        i = s;
        j = e;
        for(;;){
            while(data[j].key.searchKey >= pivot.key.searchKey && i<j){j--;}
            while(data[i].key.searchKey <= pivot.key.searchKey && i<j){i++;}
            if(i<j){
                QueryData temp = data[i];
                data[i] = data[j];
                data[j] = temp;
            }else{
                break;
            }
        }
        QueryData temp = data[i];
        data[i] = data[s];
        data[s] = temp;
        if(k<i)
            quickSelect(data, k, s, i-1);
        else if(k>i)
            quickSelect(data, k, i+1, e);
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
    memcpy(newHigh->data, leafNode->data + j,  newsize * sizeof (QueryData ));

    memset(leafNode->data + j, 0, sizeof (QueryData) * newsize);
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
    int oldSize = leafNode->node.allocated;
    if(useBFPRT){
        BFPRTSelect(leafNode->data, 0, leafNode->node.allocated - 1, median);
    } else{
        quickSelect(leafNode->data, median, 0, leafNode->node.allocated - 1);
    }

    memcpy(newHigh->data, leafNode->data + median,  (oldSize - median) * sizeof (QueryData ));

    memset(leafNode->data + median, 0, sizeof (QueryData) * (oldSize - median));
    //    for (int i = j; i < j + newsize; i++) {
    //        leafNode->node.keys[i] = NULL;
    //        // clear bound id of the influenced query
    //        leafNode->values[i] = NULL;
    //    }
    newHigh->node.allocated = (oldSize - median);
    leafNode->node.allocated = median;
    LeafNodeResetMaxValue(leafNode);
    LeafNodeResetMaxValue(newHigh);
    LeafNodeResetMinValue(leafNode);
    LeafNodeResetMinValue(newHigh);


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
        if(QueryRangeLT(leafNode->data[i].key, leafNode->data[minIndex].key)){
            minIndex = i;
        }
    }
    QueryData  temp = leafNode->data[0];
    leafNode->data[0] = leafNode->data[minIndex];
    leafNode->data[minIndex] = temp;
}

KeyType  LeafNodeSplitShiftKeysLeft(LeafNode* leafNode) {
    if(optimizationType == NoSort || optimizationType == BatchAndNoSort){
        LeafNodeResetMinKey(leafNode);
    }
    return leafNode->data[0].key;
}

int LeafNodeGetId(LeafNode* leafNode) {
    return leafNode->node.id;
}

int LeafNodeGetHeight(LeafNode* leafNode) {
    return 1;
}

void LeafNodeResetId(LeafNode* leafNode){
    for (int i = 0; i < leafNode->node.allocated ; ++i) {
        myItoa(Qid, getQueryId(leafNode->data[i].value));
        Qid ++;
    }
}


void printLeafNode(LeafNode* leafNode){
    printf("[L%d](%d)(%d,%d){", leafNode->node.id, leafNode->node.allocated,
           ((leafNode->node.minValue)), ((leafNode->node.maxValue)));
    for (int i = 0; i < leafNode->node.allocated; i++) {
        QueryRange * k = &leafNode->data[i].key;
        QueryMeta* v = leafNode->data[i].value;
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
        if(leafNode->data[i].key.lower == leafNode->node.minValue){
            findMin = TRUE;
        }
        if(leafNode->data[i].key.upper == leafNode->node.maxValue){
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
        if(leafNode->data[i].key.upper != leafNode->data[i].value->dataRegion.upper || leafNode->data[i].key.lower != leafNode->data[i].value->dataRegion.lower){
            return FALSE;
        }
    }
    return TRUE;
}


int LeafNodeFindSlotByKey( LeafNode * node, KeyType* searchKey) {
    // return Arrays.binarySearch(keys, 0, allocated, searchKey);
    if(node->node.allocated == 0){
        return -1;
    }
    int low = 0;
    int high = node->node.allocated - 1;

    while (low <= high) {
        int mid = (low + high) >> 1;
        KeyType midVal = (node->data[mid].key);

        if (QueryRangeLT(midVal, *searchKey)) {
            low = mid + 1;
        } else if (QueryRangeGT(midVal, *searchKey)) {
            high = mid - 1;
        } else {
            return mid; // key found
        }
    }
    return -(low + 1);  // key not found.
}