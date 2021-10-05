//
// Created by workshop on 8/24/2021.
//
#include <Tool/ArrayList.h>
#include <time.h>
#include <libconfig.h>
#include <pthread.h>
#include "QTree.h"

extern SearchKeyType searchKeyType;
extern u_int64_t checkLeaf;
extern u_int64_t checkQuery;
extern u_int64_t checkInternal;
extern SearchKeyType searchKeyType;

void QTreeConstructor(QTree* qTree,  int BOrder){
    memset(qTree, 0, sizeof (QTree));
    qTree->maxNodeID = 0;
    qTree->root = (Node*)malloc(sizeof (LeafNode));
    LeafNodeConstructor((LeafNode*)qTree->root, qTree);
    LeafNodeAllocId((LeafNode*)qTree->root);
    qTree->elements = 0;
    qTree->stackNodesIndex = 0;
    qTree->stackSlotsIndex = 0;
    const char ConfigFile[]= "config.cfg";

    config_t cfg;

    config_init(&cfg);

    /* Read the file. If there is an error, report it and exit. */
    if(! config_read_file(&cfg, ConfigFile))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
    }
    int  keyType;
    config_lookup_int(&cfg, "removePoint", (int*)&removePoint);
    config_lookup_int(&cfg, "searchKeyType", (int*)&keyType);
    config_lookup_int(&cfg, "optimizationType", (int*)&optimizationType);
    config_lookup_int(&cfg, "checkQueryMeta", &checkQueryMeta);

    switch (keyType) {
        case 0:
            searchKeyType = LOW;
            break;
        case  1:
            searchKeyType = DYMID;
            break;
        case 2:
            searchKeyType = Mid;
            break;
        case  3:
            searchKeyType = REMOVE;
            break;
        case  4:
            searchKeyType = RAND;
            break;
    }


//    printf("QTree searchKeyType = %d\n", searchKeyType);
//    memset(qTree->stackSlots,0, maxDepth * sizeof (int));
//    memset(qTree->stackNodes,0, maxDepth * sizeof (InternalNode *));
}

void printQTreeStatistics(QTree * qTree){
    printf("%d, %d, %d, %d,  %ld, %ld, %ld,  %ld, %ld, %ld, %ld, %ld, %ld, %ld, %d",
           Border, checkQueryMeta, optimizationType, searchKeyType, checkQuery, checkLeaf, checkInternal, checkQuery, checkLeaf, checkInternal,
               qTree->leafSplitCount, qTree->internalSplitCount, qTree->whileCount, qTree->funcCount, RemovedQueueSize);
}

void QTreeDestroy(QTree* qTree){
//    printf("searchKeyType:%d, Border:%d, leafSplitCount: %d, internalSplitCount:%d, funcTime:%ld, funcCount:%d, whileCount:%d\n",
//           searchKeyType, Border, qTree->leafSplitCount,  qTree->internalSplitCount,  qTree->funcTime,  qTree->funcCount,  qTree->whileCount);
//    NodeDestroy(qTree->root);
}

void printQTree( QTree* qTree){
    int elements_debug_local_recounter = 0;
    Node* node = NULL;
    int depth = 0;
    int height = depth;
    Node* stackNodes[1000];
    int stackNodesIndex = 0;
    int stackChildCount[1000];
    int stackChildCountIndex = 0;
    stackPush(stackNodes,stackNodesIndex, qTree->root); // init seed, root node
    BOOL lastIsInternal = (BOOL)!NodeIsLeaf(qTree->root);
    while (!(stackEmpty(stackNodes, stackNodesIndex))) {
        node = stackPop(stackNodes, stackNodesIndex);
        int count = stackPop(stackChildCount, stackChildCountIndex);
        stackPush(stackChildCount, stackChildCountIndex, count - 1);
        if (!NodeIsLeaf(node)) {
            InternalNode* internalNode = (InternalNode*) node;
            stackPush(stackChildCount, stackChildCountIndex, node->allocated + 1);
            for(int i = node->allocated; i >= 0; i --){
                stackPush(stackNodes, stackNodesIndex, internalNode->childs[i]);
            }

        } else {
            elements_debug_local_recounter += node->allocated;
        }
        // For Indentation
        if (lastIsInternal ) {
            depth += 2;
        }
        lastIsInternal = (BOOL)!NodeIsLeaf(node);
        int pad =(depth - 1);
        for (int i =0 ; i < pad; i ++){
            printf(" ");
        }
        printNode(node);
        count = stackPop(stackChildCount, stackChildCountIndex);
        while (count == 0){
            depth -= 2;
            count = stackPop(stackChildCount, stackChildCountIndex);
        }
        stackPush(stackChildCount, stackChildCountIndex, count);

    }
    printf("height=%d root=%d elements=%d recounter=%d\n", height, qTree->root->id, qTree->elements, elements_debug_local_recounter);
}

int QTreeAllocNode(QTree* qTree, BOOL isLeaf){
    int id = qTree->maxNodeID ++;
    return isLeaf ? id : -id;
}

inline void QTreeMakeNewRoot(QTree* qTree, Node* splitedNode){
    InternalNode* nodeRootNew = (InternalNode*)malloc(sizeof (InternalNode));
    InternalNodeConstructor(nodeRootNew, qTree);
    InternalNodeAllocId(nodeRootNew);

    KeyType newkey = NodeSplitShiftKeysLeft(splitedNode);
    nodeRootNew->childs[0] = qTree->root;
    nodeRootNew->node.keys[0] = newkey;
    nodeRootNew->childs[1] = splitedNode;
    nodeRootNew->node.allocated++;
    InternalNodeResetMaxValue(nodeRootNew);
    InternalNodeResetMinValue(nodeRootNew);
    qTree->root = (Node* )nodeRootNew;
}

inline void setSearchKey(Node* node, KeyType * key){
//    node->tree->funcCount ++;
    switch (searchKeyType) {
        case LOW:
            key->searchKey = key->lower;
            break;
        case DYMID:
            if(node->maxValue == 0){
                key->searchKey = (key->lower + key->upper) >> 1;
            } else if((key->lower < (node->maxValue)) && (key-> upper > (node->minValue))){
                BoundKey low = (node->minValue);
                if((key->lower > (node->minValue))){
                    low = key->lower;
                }
                BoundKey high =(node->maxValue);
                if((key->upper < (node->maxValue))){
                    high = key->upper;
                }
                key->searchKey = (low + high) >> 1;
            }
            break;
        case RAND:
            if(key->searchKey == -1){
                if(key->upper == key->lower){
                    key->searchKey = key->lower;
                }else{
                    int randNum = rand() % (key->upper - key->lower);
                    key->searchKey  = key->lower + randNum;
                }

            }
            break;
        case Mid:
            key->searchKey = (key->lower + key->upper) >> 1;
            break;
        case REMOVE:
            if(key->searchKey == -1){
                int index = clockIndex;
                for (int i = 0; i < RemovedQueueSize; ++i) {
                    index --;
                    index = index < 0? index + RemovedQueueSize: index;
                    if(QueryRangeCover(*key, RemovedKey[index])){
                        key->searchKey = RemovedKey[index];
                        break;
                    }
                }
                if(key->searchKey == -1){
                    key->searchKey = (key->lower + key->upper) >> 1;
                }
            }
            break;
    }
}

inline LeafNode* QTreeFindLeafNode(QTree* qTree, KeyType * key) {
    qTree->funcCount ++;

    Node* node = qTree->root;
    int slot = 0;

    while (!NodeIsLeaf(node)) {
        qTree->whileCount++;
        InternalNode *nodeInternal = (InternalNode*) node;
        slot = NodeFindSlotByKey(node, key);
        slot = ((slot < 0) ? (-slot) - 1 : slot + 1);

        stackPush(qTree->stackNodes, qTree->stackNodesIndex, nodeInternal);
        stackPush(qTree->stackSlots, qTree->stackSlotsIndex, slot);

        node =nodeInternal->childs[slot];
        if (node == NULL) {
            printf("ERROR childs[%d] in node %d \n", slot, nodeInternal->node.id);
            exit(-1);
        }
    }

    return (NodeIsLeaf(node) ? (LeafNode*) node : NULL);
}

void QTreePut(QTree* qTree, QueryRange * key, QueryMeta * value){
    if(key == NULL || value == NULL){
        return ;
    }
    switch (optimizationType) {
        case None:
        case NoSort:
            setSearchKey(NULL, key);
            QTreePutOne(qTree, key, value);
            return;

    }
    // empty batch
    if( qTree->batchCount == 0){
        setSearchKey(NULL, key);
        qTree->batchSearchKey = key->searchKey;
        qTree->batchKey[0] = *key;
        qTree->batchValue[0] = value;
        qTree->batchCount ++;
        qTree->batchMissCount = 0;
        return;
    }

    // put into the batch
    if( qTree->batchCount > 0 && QueryRangeCover(*key, qTree->batchSearchKey) && qTree->batchCount < MaxBatchCount){
        key->searchKey = qTree->batchSearchKey;
        int innerIndex = qTree->batchCount;
        qTree->batchKey[innerIndex] = *key;
        qTree->batchValue[innerIndex] = value;
        qTree->batchCount ++;
        if(qTree->batchCount >= MaxBatchCount){
            QTreePutBatch(qTree, qTree->batchKey, qTree->batchValue, qTree->batchCount);
            qTree->batchCount = 0;
        }
        qTree->batchMissCount = 0;
        return;
    }

    // replace the  batch or insert the key directly
    if(qTree->batchMissCount > batchMissThreshold){
        QTreePutBatch(qTree, qTree->batchKey, qTree->batchValue, qTree->batchCount);
        setSearchKey(NULL, key);
        qTree->batchSearchKey = key->searchKey;
        qTree->batchKey[0] = *key;
        qTree->batchValue[0] = value;
        qTree->batchCount = 1;
        qTree->batchMissCount = 0;
    } else{
        setSearchKey(NULL, key);
        QTreePutOne(qTree, key, value);
        qTree->batchMissCount ++;
    }

}

inline void QTreePutOne(QTree* qTree, QueryRange* key, QueryMeta* value){
    if(key == NULL || value == NULL){
        return ;
    }

    LeafNode* nodeLeaf = QTreeFindLeafNode(qTree, key);
    if (nodeLeaf == NULL) {
        printf("QTreeFindLeafNode error!\n");
        exit(-1);
    }
    BoundKey min = key->lower, max = key->upper;
    int slot;
    switch (optimizationType) {
        case None:
            // Find in leaf node for key
            slot = NodeFindSlotByKey((Node*)nodeLeaf, key);

            slot = (slot >= 0)?(slot + 1):((-slot) - 1);

            LeafNodeAdd(nodeLeaf, slot, key, value);
            break;
        case NoSort:
        case BatchAndNoSort:
            LeafNodeAddLast(nodeLeaf, key, value);
            break;
        default:
            printf("unSupport type:%d\n", optimizationType);
            exit(-1);
    }
    //

    Node*   splitedNode = (NodeIsFull((Node*)nodeLeaf) ? LeafNodeSplit(nodeLeaf) : NULL);

    // Iterate back over nodes checking overflow / splitting
    while (!stackEmpty(qTree->stackNodes, qTree->stackNodesIndex)) {
        //        cout << slot << endl;
        InternalNode* node = stackPop(qTree->stackNodes, qTree->stackNodesIndex);
        if((max >(node->node.maxValue)) ){
            node->node.maxValue = max;
        }
        if( ((node->node.minValue) > min) ){
            node->node.minValue = min;
        }

        slot = stackPop(qTree->stackSlots, qTree->stackSlotsIndex);
        //            System.out.println(key + ", "  + otherBound + "," + node.id + ", "  + node.keys[0] + "," + slot);
        if (splitedNode != NULL) {
            // split occurred in previous phase, splitedNode is new child
            KeyType  childKey = NodeSplitShiftKeysLeft(splitedNode);
            InternalNodeAdd(node, slot, &childKey, splitedNode);

        }
        if(NodeIsFull((Node*)node)){
            splitedNode = InternalNodeSplit(node);
        }else{
            splitedNode = NULL;
        }
        //    splitedNode = (node->isFull() ? node->split() : NULL);
    }

    qTree->elements += 1;
    if (splitedNode != NULL) {   // root was split, make new root
        QTreeMakeNewRoot(qTree, splitedNode);
    }
    //    NodeCheckTree(qTree->root);
}

inline void QTreePutBatch(QTree* qTree, QueryRange key[], QueryMeta* value[], int batchCount){
    if(key == NULL || value == NULL){
        return ;
    }

    LeafNode* nodeLeaf = QTreeFindLeafNode(qTree, &key[0]);
    if (nodeLeaf == NULL) {
        printf("QTreeFindLeafNode error!\n");
        exit(-1);
    }
    int inserted = 0;
    BoundKey min = key[0].lower, max = key[0].upper;
    for(; inserted < batchCount && (!NodeIsFull((Node*)nodeLeaf)); inserted ++ ){
        // unsorted leaf, insert into the End
        LeafNodeAddLast(nodeLeaf, key + inserted, value[inserted]);
        if(key[inserted].lower < min){
            min = key[inserted].lower;
        }
        if(key[inserted].upper > max){
            max = key[inserted].upper;
        }
    }
    Node*   splitedNode = (NodeIsFull((Node*)nodeLeaf) ? LeafNodeSplit(nodeLeaf) : NULL);

    if(splitedNode != NULL && inserted < batchCount){ // something remains in the batch
        LeafNodeResetMinKey((LeafNode* )splitedNode);
        LeafNode* insertToNode;
        if(QueryRangeLT(key[inserted], splitedNode->keys[0])){
            insertToNode = nodeLeaf;
        } else{
            insertToNode = (LeafNode* )splitedNode;
        }
        // unable to make the node full
        for (; inserted < batchCount ; inserted ++) {
            LeafNodeAddLast(insertToNode, key + inserted, value[inserted]);
            if(key[inserted].lower < min){
                min = key[inserted].lower;
            }
            if(key[inserted].upper > max){
                max = key[inserted].upper;
            }
        }
    }

    // Iterate back over nodes checking overflow / splitting
    int slot;
    while (!stackEmpty(qTree->stackNodes, qTree->stackNodesIndex)) {
        //        cout << slot << endl;
        InternalNode* node = stackPop(qTree->stackNodes, qTree->stackNodesIndex);
        if((max >(node->node.maxValue)) ){
            node->node.maxValue = max;
        }
        if( ((node->node.minValue) > min) ){
            node->node.minValue = min;
        }
        slot = stackPop(qTree->stackSlots, qTree->stackSlotsIndex);
        //            System.out.println(key + ", "  + otherBound + "," + node.id + ", "  + node.keys[0] + "," + slot);
        if (splitedNode != NULL) {
            // split occurred in previous phase, splitedNode is new child
            KeyType  childKey = NodeSplitShiftKeysLeft(splitedNode);
            InternalNodeAdd(node, slot, &childKey, splitedNode);

        }
        if(NodeIsFull((Node*)node)){
            splitedNode = InternalNodeSplit(node);
        }else{
            splitedNode = NULL;
        }
        //    splitedNode = (node->isFull() ? node->split() : NULL);
    }

    qTree->elements += batchCount;
    if (splitedNode != NULL) {   // root was split, make new root
        QTreeMakeNewRoot(qTree, splitedNode);
    }
//    NodeCheckTree(qTree->root);
}



//delete queries in the batch queue
inline void QTreeCheckBatch(QTree* qTree, int attribute, Arraylist* removedQuery){
    int newCount = 0;
    for (int j = 0; j < qTree->batchCount; ++j) {
        if(QueryRangeCover(qTree->batchKey[j], attribute)){
            ArraylistAdd(removedQuery, qTree->batchValue[j]);
        } else{
            qTree->batchKey[newCount] = qTree->batchKey[j];
            qTree->batchValue[newCount ++] =  qTree->batchValue[j];
        }
    }
    qTree->batchCount = newCount;
}

inline Node* checkInternalNode(QTree* qTree, InternalNode* nodeInternal,  KeyType* key){
    checkInternal ++;
    Node* node = NULL;
    for(int slot = 0; slot <= nodeInternal->node.allocated; slot ++){
        if(((nodeInternal->childs[slot]->maxValue) >= key->lower) && ((nodeInternal->childs[slot]->minValue) <= key->upper)){
            node = nodeInternal->childs[slot];
            stackPush(qTree->stackNodes, qTree->stackNodesIndex, nodeInternal);
            stackPush(qTree->stackSlots, qTree->stackSlotsIndex, slot);
            break;
        }
    }
    return node;
}

inline BOOL CheckLeafNodeCover(LeafNode * leafNode, int i,  BoundKey attribute){
    if(checkQueryMeta){
        return QueryRangeCover (((LeafNode*)leafNode)->node.keys[i], attribute);
    }else{
        return  QueryMetaCover(((LeafNode*)leafNode)->values[i], attribute);
    }
}

inline void checkLeafNode(QTree* qTree, LeafNode* leafNode, BoundKey* removedMax, BoundKey* removedMin, BoundKey attribute, Arraylist* removedQuery){
    int j = 0;
    BOOL resetMax = FALSE;
    BOOL resetMin = FALSE;
    for(int i = 0; i < leafNode->node.allocated ; i ++){
        checkQuery ++;
        //                    System.out.println("query:" + leafNode.values[i]);
        if(CheckLeafNodeCover(leafNode,i, attribute)){

            if(leafNode->node.keys[i].upper >= leafNode->node.maxValue){
                resetMax = TRUE;
            }
            if(leafNode->node.keys[i].lower <= leafNode->node.minValue){
                resetMin = TRUE;
            }
            ArraylistAdd(removedQuery, leafNode->values[i]);
            qTree->elements --;
        }else {
            leafNode->node.keys[j] = leafNode->node.keys[i];
            leafNode->values[j ++] = leafNode->values[i];
        }
    }
    leafNode->node.allocated = j;
    if(resetMax){
        if(*removedMax < leafNode->node.maxValue){
            *removedMax = leafNode->node.maxValue;
        }
        LeafNodeResetMaxValue(leafNode);
    }
    if(resetMin){
        if(*removedMin > leafNode->node.minValue){
            *removedMin = leafNode->node.minValue;
        }
        LeafNodeResetMinValue(leafNode);
    }
}

inline Node* getAnotherNode(QTree* qTree, KeyType* key, BoundKey removedMax, BoundKey removedMin){
    Node* node = NULL;
    int slot;
    while (!stackEmpty(qTree->stackNodes, qTree->stackNodesIndex)){
        node = (Node*)stackPop(qTree->stackNodes, qTree->stackNodesIndex);
        slot = stackPop(qTree->stackSlots, qTree->stackSlotsIndex);

        InternalNode* internalNode = (InternalNode*) node;
        for (; slot < node->allocated; slot ++) {
            if(((internalNode->childs[slot + 1]->maxValue) >= key->lower) && ((internalNode->childs[slot + 1]->minValue) <= key->upper)){
                node = internalNode->childs[slot + 1];
                stackPush(qTree->stackNodes, qTree->stackNodesIndex, internalNode);
                stackPush(qTree->stackSlots, qTree->stackSlotsIndex, slot + 1);
                return node;
            }
        }

        if(slot >= node->allocated ){
            for(int i = 0; i < slot; i ++ ){
                InternalNodeCheckUnderflowWithRight(((InternalNode*) node), i);
            }
            if(node->maxValue <= removedMax){
                InternalNodeResetMaxValue(((InternalNode*) node));
            }
            if(node->minValue >= removedMin){
                InternalNodeResetMinValue(((InternalNode*) node));
            }
        }
        node = NULL;
    }
    return node;
}



void QTreeFindAndRemoveRelatedQueries(QTree* qTree, int attribute, Arraylist* removedQuery){
    if(searchKeyType == REMOVE){
        for (int i = 0; i < RemovedQueueSize; ++i) {
            clockIndex = (clockIndex + 1) % RemovedQueueSize;
            if(RemovedKey[clockIndex] == attribute){
                break;
            }
            if(clockFlag & (1 << clockIndex)){
                clockFlag &= ~(1 << clockIndex);
            } else{
                RemovedKey[clockIndex] = attribute;
                clockFlag |= (1 << clockIndex);
                break;
            }
        }
    }
//    if(checkInternal > 20){
//        printf("checkInternal\n");
//    }
    int oldCheckInternal = checkInternal, oldRemove = removedQuery->size;
    checkInternal = 0;
    if(optimizationType == BatchAndNoSort){
        QTreeCheckBatch(qTree, attribute, removedQuery);
    }
    Node* node = qTree->root;
    KeyType  queryRange;
    KeyType* key = &queryRange;
    QueryRangeConstructorWithPara(key, attribute, attribute, TRUE, TRUE);
    BoundKey removedMax = 0, removedMin = maxValue << 1;
    while (TRUE) {
        while (!NodeIsLeaf(node)){
            InternalNode* nodeInternal = (InternalNode*) node;
            node = checkInternalNode( qTree, nodeInternal,   key);
            //                System.out.println("getNode:" + getNode + ", node:" + node);
            if(node == NULL){
                node = getAnotherNode(qTree, key, removedMax, removedMin);
                if(node == NULL){
                    break;
                }
            }
        }
        if(node == NULL){
            break;
        }
        if(NodeIsLeaf(node)){
            checkLeaf ++;
            LeafNode* leafNode = (LeafNode*) node;
            //                System.out.println("getLeafNode:" + leafNode);
            checkLeafNode(qTree, leafNode,  &removedMax, &removedMin, attribute, removedQuery);
            if(stackEmpty(qTree->stackNodes, qTree->stackNodesIndex)){
                break;
            }
            node = getAnotherNode(qTree, key, removedMax, removedMin);
            if(node == NULL){
                break;
            }
        }else {
            break;
        }
    }
    checkInternal = checkInternal / (removedQuery->size - oldRemove + 1);
    if(checkInternal < oldCheckInternal){
        checkInternal = oldCheckInternal;
    }
//    NodeCheckTree(qTree->root);
}


BOOL QTreeCheckMaxMin(QTree* qTree){
    return NodeCheckMaxMin(qTree->root);
}