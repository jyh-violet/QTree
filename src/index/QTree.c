//
// Created by workshop on 8/24/2021.
//
#include <Tool/ArrayList.h>
#include <time.h>
#include <libconfig.h>
#include <pthread.h>
#include <unistd.h>
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
    pthread_spin_init(&qTree->removeLock, PTHREAD_PROCESS_SHARED);
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
    printf("%d, %d, %d, %d,  %ld, %ld, %ld, %ld, %ld, %ld, %ld, %d, %d, ",
           Border, checkQueryMeta, optimizationType, searchKeyType, checkQuery, checkLeaf, checkInternal,
               qTree->leafSplitCount, qTree->internalSplitCount, qTree->whileCount, qTree->funcCount, RemovedQueueSize, setKeyCount);
}

void QTreeResetStatistics(QTree* qTree){
    qTree->leafSplitCount = 0;
    qTree->internalSplitCount = 0;
    qTree->whileCount = 0;
    qTree->funcCount = 0;
    setKeyCount = 0;
}

void QTreeDestroy(QTree* qTree){
    pthread_spin_destroy(&qTree->removeLock);
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
    nodeRootNew->keys[0] = newkey;
    nodeRootNew->childs[1] = splitedNode;
    nodeRootNew->node.allocated++;
    InternalNodeResetMaxValue(nodeRootNew);
    InternalNodeResetMinValue(nodeRootNew);
    qTree->root = (Node* )nodeRootNew;
    NodeRmRemoveReadInsertWriteLock(splitedNode);
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
                        setKeyCount ++;
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

inline BOOL QTreeAddLockForFindLeaf(Node* node, int threadId){
    if(NodeIsLeaf(node)){
        return NodeTryAddInsertWriteLock(node);
    } else{
        return NodeTryAddInsertReadLock(node, threadId);
    }
}

inline void QTreeRmLockForFindLeaf(Node* node, int threadId){
    if(node ==NULL){
        return;
    }
    if(NodeIsLeaf(node)){
        NodeRmInsertWriteLock(node);
    } else{
        NodeRmInsertReadLockNoLog(node, threadId);
    }
}

inline BOOL QTreeModifyNodeMaxMin(Node* node, BoundKey min, BoundKey max){
    BOOL restMaxMin = FALSE;
    BoundKey oldBound = __sync_fetch_and_add (&node->maxValue, 0);
    while (oldBound < max ){
        if(__sync_bool_compare_and_swap (&node->maxValue,oldBound, max)){
            restMaxMin = TRUE;
            break;
        } else{
            oldBound = __sync_fetch_and_add (&node->maxValue, 0);
        }

    }
    oldBound =  __sync_fetch_and_add (&node->minValue, 0);
    while( (oldBound > min) ){
        if( (__sync_bool_compare_and_swap (&node->minValue,oldBound, min) ) ){
            restMaxMin = TRUE;
            break;
        } else{
            oldBound =  __sync_fetch_and_add (&node->minValue, 0);
        }

    }
    return restMaxMin;
}

inline Node* QTreeTravelRightLink(Node* node, KeyType * key, int threadId){
    while (key->searchKey > node->nextNodeMin){
        NodeAddRemoveReadLock( node->right);
        if(QTreeAddLockForFindLeaf(node->right, threadId) == FALSE){
            NodeRmRemoveReadLock(node->right);
            QTreeRmLockForFindLeaf(node, threadId);
            NodeRmRemoveReadLock(node);
            return NULL;
        }
        Node* temp = node;
        node =  node->right;
        QTreeRmLockForFindLeaf(temp, threadId);
        NodeRmRemoveReadLock(temp);
    }
    return node;
}

inline LeafNode* QTreeFindLeafNode(QTree* qTree, KeyType * key, NodesStack* nodesStack, IntStack* slotStack, BoundKey min, BoundKey max, int threadId) {
//    qTree->funcCount ++;
    Node* node  = NULL;
    int findTime = 0;
    findAgain:{
        findTime ++;
        if(findTime % 1000 == 0){
            vmlog(WARN,"QTreeFindLeafNode retry:%d", findTime);
            if(findTime % 10000 == 0){
                while (!stackEmpty(nodesStack->stackNodes, nodesStack->stackNodesIndex)){
                    node = (Node*)stackPop(nodesStack->stackNodes, nodesStack->stackNodesIndex);
                    vmlog(WARN, "stackNode: %d", node->id);
                }
                exit(-1);
            }
        }
        int sleep = 0;
        while (!stackEmpty(nodesStack->stackNodes, nodesStack->stackNodesIndex)){
            node = (Node*)stackPop(nodesStack->stackNodes, nodesStack->stackNodesIndex);
            QTreeRmLockForFindLeaf(node,threadId);
            NodeRmRemoveReadLock(node);
            sleep = 1;
        }
        if(sleep){
            usleep(100);
        }
        node = qTree->root;
        int slot = 0;
        NodeAddRemoveReadLock(node);
        if(QTreeAddLockForFindLeaf(node, threadId) == FALSE){
            NodeRmRemoveReadLock(node);
            goto  findAgain;
        }
        node = QTreeTravelRightLink(node, key, threadId);
        if(node == NULL){
            goto findAgain;
        }
        while (!NodeIsLeaf(node)) {
            InternalNode *nodeInternal = (InternalNode*) node;
            stackPush(nodesStack->stackNodes, nodesStack->stackNodesIndex, nodeInternal);

            slot = InternalNodeFindSlotByKey(nodeInternal, key);
            slot = ((slot < 0) ? (-slot) - 1 : slot + 1);
            node =nodeInternal->childs[slot];
            NodeAddRemoveReadLock(node);
            if(QTreeAddLockForFindLeaf(node, threadId) == FALSE){
                NodeRmRemoveReadLock(node);
                goto  findAgain;
            }
            node = QTreeTravelRightLink(node, key, threadId);
            if(node == NULL || !NodeIsValid(node)){
                QTreeRmLockForFindLeaf(node, threadId);
                NodeRmRemoveReadLock(node);
                goto findAgain;
            }
            QTreeRmLockForFindLeaf((Node*)nodeInternal, threadId);
        }

    }

    return (NodeIsLeaf(node) ? (LeafNode*) node : NULL);
}

void QTreePut(QTree* qTree, QueryRange * key, QueryMeta * value, int threadId){
    if(key == NULL || value == NULL){
        return ;
    }
    switch (optimizationType) {
        case None:
        case NoSort:
            setSearchKey(NULL, key);
            QTreePutOne(qTree, key, value, threadId);
            return;

    }
    // empty batch
    if( qTree->batchCount == 0){
        setSearchKey(NULL, key);
        qTree->batchSearchKey = key->searchKey;
        qTree->batch[0].key = *key;
        qTree->batch[0].value = value;
        qTree->batchCount ++;
        qTree->batchMissCount = 0;
        return;
    }

    // put into the batch
    if( qTree->batchCount > 0 && QueryRangeCover(*key, qTree->batchSearchKey) && qTree->batchCount < MaxBatchCount){
        key->searchKey = qTree->batchSearchKey;
        int innerIndex = qTree->batchCount;
        qTree->batch[innerIndex].key = *key;
        qTree->batch[innerIndex].value = value;
        qTree->batchCount ++;
        if(qTree->batchCount >= MaxBatchCount){
            QTreePutBatch(qTree, qTree->batch, qTree->batchCount, threadId);
            qTree->batchCount = 0;
        }
        qTree->batchMissCount = 0;
        return;
    }

    // replace the  batch or insert the key directly
    if(qTree->batchMissCount > batchMissThreshold){
        QTreePutBatch(qTree, qTree->batch, qTree->batchCount, threadId);
        setSearchKey(NULL, key);
        qTree->batchSearchKey = key->searchKey;
        qTree->batch[0].key = *key;
        qTree->batch[0].value = value;
        qTree->batchCount = 1;
        qTree->batchMissCount = 0;
    } else{
        setSearchKey(NULL, key);
        QTreePutOne(qTree, key, value, threadId);
        qTree->batchMissCount ++;
    }

}

inline void QTreePutOne(QTree* qTree, QueryRange* key, QueryMeta* value, int threadId){
    if(key == NULL || value == NULL){
        return ;
    }

    NodesStack  nodesStack;
    IntStack slotStack;
    nodesStack.stackNodesIndex = 0;
    slotStack.stackSlotsIndex = 0;
    BoundKey min = key->lower, max = key->upper;
    LeafNode* nodeLeaf;
    int slot;
    nodeLeaf = QTreeFindLeafNode(qTree, key, &nodesStack, &slotStack, min, max, threadId);

    BOOL restMaxMin = FALSE;
    switch (optimizationType) {
        case None:
        case Batch:
            // Find in leaf node for key
            slot = LeafNodeFindSlotByKey(nodeLeaf, key);

            slot = (slot >= 0)?(slot + 1):((-slot) - 1);

            restMaxMin = LeafNodeAdd(nodeLeaf, slot, key, value);
            break;
        case NoSort:
        case BatchAndNoSort:
            restMaxMin = LeafNodeAddLast(nodeLeaf, key, value);
            break;
        default:
            printf("QTreePutOne: unSupport type:%d\n", optimizationType);
            exit(-1);
    }
    //

    Node*   splitedNode = (NodeIsFull((Node*)nodeLeaf) ? LeafNodeSplit(nodeLeaf) : NULL);
    Node*   lastNode = (Node*)nodeLeaf;
    if(splitedNode == NULL){
        NodeRmRemoveReadInsertWriteLock(lastNode);
    }

    // Iterate back over nodes checking overflow / splitting
    while (!stackEmpty(nodesStack.stackNodes, nodesStack.stackNodesIndex)) {
        //        cout << slot << endl;
        InternalNode* node = stackPop(nodesStack.stackNodes, nodesStack.stackNodesIndex);
//        vmlog(InsertLog, "QTreePutOne, stackPop node:%d", node->node.id);

        if (splitedNode != NULL) {
            NodeAddInsertWriteLock((Node*)node);
            // split occurred in previous phase, splitedNode is new child
            KeyType  childKey = NodeSplitShiftKeysLeft(splitedNode);
            slot = -1;
            while ((slot = InternalNodeFindSlotByChild(node, lastNode)) < 0){
                NodeAddRemoveReadInsertWriteLock(node->node.right);
                Node* tempNode = (Node*)node;
                node = (InternalNode*) node->node.right;
                NodeRmRemoveReadInsertWriteLock(tempNode);
            }
            InternalNodeAdd(node, slot, &childKey, splitedNode);
            min = splitedNode->minValue < min? splitedNode->minValue: min;
            max = splitedNode->maxValue > max? splitedNode->maxValue: max;
            min = lastNode->minValue < min? lastNode->minValue: min;
            max = lastNode->maxValue > max? lastNode->maxValue: max;
            restMaxMin = QTreeModifyNodeMaxMin( (Node*)node, min, max);
            NodeRmRemoveReadInsertWriteLock(splitedNode);
            NodeRmRemoveReadInsertWriteLock(lastNode);
            if(NodeIsFull((Node*)node)){
                splitedNode = InternalNodeSplit(node);
            }else if(restMaxMin){
                NodeRmRemoveReadLock((Node*)node);
                NodeDegradeInsertLock((Node*)node, threadId);
                splitedNode = NULL;
            }else{
                NodeRmRemoveReadInsertWriteLock((Node*)node);
                splitedNode = NULL;
            }
        } else if(restMaxMin){
            NodeAddInsertReadLock((Node*)node, threadId);
            NodeRmRemoveReadLock((Node*)node);
            while ((InternalNodeFindSlotByChild(node, lastNode)) < 0){
                NodeAddInsertReadLock(node->node.right, threadId);
                Node* tempNode = (Node*)node;
                node = (InternalNode*) node->node.right;
                NodeRmInsertReadLock(tempNode, threadId);
            }
            restMaxMin = QTreeModifyNodeMaxMin( (Node*)node, min, max);
            NodeRmInsertReadLock(lastNode, threadId);
            if(!restMaxMin){
                NodeRmInsertReadLock( (Node*)node, threadId);
            }
        } else{
            NodeRmRemoveReadLock((Node*)node);
        }
        lastNode = (Node*) node;
        //    splitedNode = (node->isFull() ? node->split() : NULL);
    }

    //handle root
    if (splitedNode != NULL) {   // root was split, make new root
        if(lastNode ==  qTree->root){
            QTreeMakeNewRoot(qTree, splitedNode);
        } else{
            while (lastNode != qTree->root){
                InternalNode* node = (InternalNode*) qTree->root;
                NodeAddRemoveReadInsertWriteLock((Node*)node);
                KeyType  childKey = NodeSplitShiftKeysLeft(splitedNode);
                while ((slot = InternalNodeFindSlotByChild(node, lastNode)) < 0){
                    if(node ->node.right == NULL){
                        vmlog(WARN, "ERROR: root split: miss max: %d, min:%d", max, min);
                        NodeAddRemoveReadInsertWriteLock((Node*)node);
                        break;
                    }
                    NodeAddRemoveReadInsertWriteLock(node->node.right);
                    Node* tempNode = (Node*)node;
                    node = (InternalNode*) node->node.right;
                    NodeRmRemoveReadInsertWriteLock(tempNode);
                }
                if(splitedNode != NULL){
                    InternalNodeAdd(node, slot, &childKey, splitedNode);
                }
                if(restMaxMin){
                    restMaxMin = QTreeModifyNodeMaxMin( (Node*)node, min, max);
                }
                NodeRmRemoveReadInsertWriteLock(splitedNode);
                NodeRmRemoveReadInsertWriteLock(lastNode);
                splitedNode = NULL;
                lastNode = (Node*) node;
            }
        }
        NodeRmRemoveReadInsertWriteLock(lastNode);
    } else if(lastNode != qTree->root && restMaxMin){
        while (lastNode != qTree->root && restMaxMin){
            InternalNode* node = (InternalNode*) qTree->root;
            NodeAddInsertReadLock((Node*)node, threadId);

            while ((InternalNodeFindSlotByChild(node, lastNode)) < 0){
                if(node ->node.right == NULL){
                    vmlog(WARN, "ERROR: root split: miss max: %d, min:%d", max, min);
                    NodeRmInsertReadLock((Node*)node, threadId);
                    break;
                }
                NodeAddInsertReadLock(node->node.right, threadId);
                Node* tempNode = (Node*)node;
                node = (InternalNode*) node->node.right;
                NodeRmInsertReadLock(tempNode, threadId);
            }
            restMaxMin = QTreeModifyNodeMaxMin( (Node*)node, min, max);
            NodeRmInsertReadLock(lastNode, threadId);
            lastNode = (Node*) node;
        }
        NodeRmInsertReadLock(lastNode, threadId);
    } else if(restMaxMin){
        NodeRmInsertReadLock(lastNode, threadId);
    }

    qTree->elements ++;

    //    NodeCheckTree(qTree->root);
}

inline void QTreePutBatch(QTree* qTree, QueryData * batch, int batchCount, int threadId){
    if(batchCount == 0){
        return ;
    }

    NodesStack  nodesStack;
    IntStack slotStack;
    nodesStack.stackNodesIndex = 0;
    slotStack.stackSlotsIndex = 0;
    BoundKey min = batch[0].key.lower, max = batch[0].key.upper;
    for (int i = 0; i < batchCount; ++i) {
        if(batch[i].key.lower < min){
            min = batch[i].key.lower;
        }
        if(batch[i].key.upper > max){
            max = batch[i].key.upper;
        }
    }

    LeafNode* nodeLeaf = QTreeFindLeafNode(qTree, &batch[0].key, &nodesStack, &slotStack, min, max, threadId);
    if (nodeLeaf == NULL) {
        printf("QTreeFindLeafNode error!\n");
        exit(-1);
    }

    Node*   splitedNode ;
    int slot;
    switch (optimizationType) {
        case BatchAndNoSort:{
            int inserted = 0;
            
            for(; inserted < batchCount && (!NodeIsFull((Node*)nodeLeaf)); inserted ++ ){
                // unsorted leaf, insert into the End
                LeafNodeAddLast(nodeLeaf, &batch[inserted].key, batch[inserted].value);
            }
            
            splitedNode = (NodeIsFull((Node*)nodeLeaf) ? LeafNodeSplit(nodeLeaf) : NULL);

            if(splitedNode != NULL && inserted < batchCount){ // something remains in the batch
//                LeafNodeResetMinKey((LeafNode* )splitedNode);
                LeafNode* insertToNode;
                if(QueryRangeLT(batch[inserted].key, (((LeafNode* )splitedNode)->data[0].key))){
                    insertToNode = nodeLeaf;
                } else{
                    insertToNode = (LeafNode* )splitedNode;
                }
                // unable to make the node full
                for (; inserted < batchCount ; inserted ++) {
                    LeafNodeAddLast(insertToNode, &batch[inserted].key, batch[inserted].value);
                }
            }
            break;
        }
        case Batch:{
            // Find in leaf node for key
            slot = LeafNodeFindSlotByKey(nodeLeaf, &batch[0].key);
            slot = (slot >= 0)?(slot + 1):((-slot) - 1);
            int insertBatchCount = nodeLeaf->node.allocated + batchCount <= Border? batchCount : (Border - nodeLeaf->node.allocated);
            LeafNodeAddBatch(nodeLeaf, slot, batch, insertBatchCount, &min, &max);
            splitedNode = (NodeIsFull((Node*)nodeLeaf) ? LeafNodeSplit(nodeLeaf) : NULL);
            if(splitedNode != NULL && insertBatchCount < batchCount){
                LeafNode* insertToNode;
                if(QueryRangeLT(batch[insertBatchCount].key, (((LeafNode* )splitedNode)->data[0].key))){
                    insertToNode = nodeLeaf;
                } else{
                    insertToNode = (LeafNode* )splitedNode;
                }
                slot = LeafNodeFindSlotByKey(insertToNode, &batch[0].key);
                slot = (slot >= 0)?(slot + 1):((-slot) - 1);
                BoundKey minTemp = batch[insertBatchCount].key.lower, maxTemp = batch[insertBatchCount].key.upper;
                LeafNodeAddBatch(insertToNode, slot, batch + insertBatchCount, batchCount - insertBatchCount, &minTemp, &maxTemp);
            }
            break;
        }
        default:
            printf("QTreePutBatch: unSupport type:%d\n", optimizationType);
            exit(-1);
    }
    
    

    // Iterate back over nodes checking overflow / splitting
  
    while (!stackEmpty(nodesStack.stackNodes, nodesStack.stackNodesIndex)) {
        //        cout << slot << endl;
        InternalNode* node = stackPop(nodesStack.stackNodes, nodesStack.stackNodesIndex);
        if((max >(node->node.maxValue)) ){
            node->node.maxValue = max;
        }
        if( ((node->node.minValue) > min) ){
            node->node.minValue = min;
        }
        slot = stackPop(slotStack.stackSlots, slotStack.stackSlotsIndex);
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
        if(QueryRangeCover(qTree->batch[j].key, attribute)){
            ArraylistAdd(removedQuery, qTree->batch[j].value);
        } else{
            qTree->batch[newCount ++] = qTree->batch[j];
        }
    }
    qTree->batchCount = newCount;
}

inline Node* checkInternalNode(QTree* qTree, InternalNode* nodeInternal,  KeyType* key, NodesStack *nodesStack, IntStack* slotStack){
    checkInternal ++;
    Node* node = NULL;
    NodeAddRemoveReadInsertWriteLock((Node*)nodeInternal);
    for(int slot = 0; slot <= nodeInternal->node.allocated; slot ++){
//        NodeAddRWLock(nodeInternal->childs[slot]);
        if(((nodeInternal->childs[slot]->maxValue) >= key->lower) && ((nodeInternal->childs[slot]->minValue) <= key->upper)){
            node = nodeInternal->childs[slot];
            stackPush(nodesStack->stackNodes, nodesStack->stackNodesIndex, nodeInternal);
            stackPush(slotStack->stackSlots, slotStack->stackSlotsIndex, slot);
//            NodeRmRWLock(nodeInternal->childs[slot]);
            break;
        }
//        NodeRmRWLock(nodeInternal->childs[slot]);
    }
    NodeRmRemoveReadInsertWriteLock((Node*)nodeInternal);
    return node;
}

inline BOOL CheckLeafNodeCover(LeafNode * leafNode, int i,  BoundKey attribute){
    if(checkQueryMeta){
        return QueryMetaCover(((LeafNode*)leafNode)->data[i].value, attribute);
    }else{
        return  QueryRangeCover (((LeafNode*)leafNode)->data[i].key, attribute);
    }
}

inline void checkLeafNode(QTree* qTree, LeafNode* leafNode, BoundKey* removedMax, BoundKey* removedMin, BoundKey attribute, Arraylist* removedQuery){
    int j = 0;
    BOOL resetMax = FALSE;
    BOOL resetMin = FALSE;
    NodeAddRemoveReadInsertWriteLock((Node*)leafNode);
    for(int i = 0; i < leafNode->node.allocated ; i ++){
        checkQuery ++;
        //                    System.out.println("query:" + leafNode.values[i]);
        if(CheckLeafNodeCover(leafNode,i, attribute)){

            if(leafNode->data[i].key.upper >= leafNode->node.maxValue){
                resetMax = TRUE;
            }
            if(leafNode->data[i].key.lower <= leafNode->node.minValue){
                resetMin = TRUE;
            }
            if(*removedMax < leafNode->data[i].key.upper){
                *removedMax = leafNode->data[i].key.upper;
            }
            if(*removedMin > leafNode->data[i].key.lower){
                *removedMin = leafNode->data[i].key.lower;
            }
            ArraylistAdd(removedQuery, leafNode->data[i].value);
            qTree->elements --;
        }else {
            leafNode->data[j ++] = leafNode->data[i];
        }
    }
    leafNode->node.allocated = j;
    if(resetMax){

        LeafNodeResetMaxValue(leafNode);
    }
    if(resetMin){

        LeafNodeResetMinValue(leafNode);
    }
    NodeRmRemoveReadInsertWriteLock((Node*)leafNode);
}

inline Node* getAnotherNode(QTree* qTree, KeyType* key, BoundKey* removedMax, BoundKey* removedMin, Arraylist* removedQuery, NodesStack *nodesStack, IntStack* slotStack){
    Node* node = NULL;
    int slot;

    while (!stackEmpty(nodesStack->stackNodes, nodesStack->stackNodesIndex)){
        node = (Node*)stackPop(nodesStack->stackNodes, nodesStack->stackNodesIndex);
        slot = stackPop(slotStack->stackSlots, slotStack->stackSlotsIndex);

        InternalNode* internalNode = (InternalNode*) node;
        NodeAddInsertWriteLock((Node*)internalNode);
        for (; slot < node->allocated; slot ++) {
//            NodeAddRWLock(internalNode->childs[slot]);
            if(((internalNode->childs[slot + 1]->maxValue) >= key->lower) && ((internalNode->childs[slot + 1]->minValue) <= key->upper)){
                node = internalNode->childs[slot + 1];
                stackPush(nodesStack->stackNodes, nodesStack->stackNodesIndex, internalNode);
                stackPush(slotStack->stackSlots, slotStack->stackSlotsIndex, slot + 1);
//                NodeRmRWLock(internalNode->childs[slot]);
                NodeRmInsertWriteLock((Node*)internalNode);
                return node;
            }
//            NodeRmRWLock(internalNode->childs[slot]);
        }
        BOOL childMerge = FALSE;
        BOOL childIsLeaf = NodeIsLeaf(internalNode->childs[0]);
        NodeRmInsertWriteLock((Node*)internalNode);
        InternalNodeAddRemoveLock(internalNode);
        for(int i = 0; i <= node->allocated; i ++ ){
            NodeAddInsertWriteLock(internalNode->childs[i]);
            while ((i <= node->allocated) && ((!childIsLeaf && internalNode->childs[i]->allocated < 0) || (childIsLeaf && internalNode->childs[i]->allocated == 0))){
                if(i > 0){
                    NodeAddInsertWriteLock(internalNode->childs[i - 1]);
                    internalNode->childs[i - 1]->right = internalNode->childs[i]->right;
                    internalNode->childs[i - 1]->nextNodeMin = internalNode->childs[i]->nextNodeMin;
                    NodeRmInsertWriteLock(internalNode->childs[i - 1]);
                }
                InternalNodeRemove(internalNode, i - 1);
                if(i <= node->allocated){
                    NodeAddInsertWriteLock(internalNode->childs[i]);
                }
                childMerge = TRUE;
            }
            if(i <= node->allocated){
                NodeRmInsertWriteLock(internalNode->childs[i]);
                childMerge = InternalNodeCheckUnderflowWithRight(((InternalNode*) node), i) || childMerge;
            }
        }
        if(node->maxValue <= *removedMax || childMerge){
            if(*removedMax < node->maxValue){
                *removedMax = node->maxValue;
            }
            InternalNodeResetMaxValue(((InternalNode*) node));
        }
        if(node->minValue >= *removedMin || childMerge){
            if(*removedMin > node->minValue){
                *removedMin = node->minValue;
            }
            InternalNodeResetMinValue(((InternalNode*) node));
        }
        InternalNodeRmRemoveLock(internalNode);
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
    pthread_spin_lock(&qTree->removeLock);
    NodesStack  nodesStack;
    IntStack slotStack;
    nodesStack.stackNodesIndex = 0;
    slotStack.stackSlotsIndex = 0;
    int oldCheckInternal = checkInternal, oldRemove = removedQuery->size;
    checkInternal = 0;
    if(optimizationType == BatchAndNoSort || optimizationType == Batch){
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
            node = checkInternalNode( qTree, nodeInternal,   key, &nodesStack, &slotStack);
            //                System.out.println("getNode:" + getNode + ", node:" + node);
            if(node == NULL){
                node = getAnotherNode(qTree, key, &removedMax, &removedMin, removedQuery, &nodesStack, &slotStack);
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
            if(stackEmpty(nodesStack.stackNodes, nodesStack.stackNodesIndex)){
                break;
            }
            node = getAnotherNode(qTree, key, &removedMax, &removedMin, removedQuery, &nodesStack, &slotStack);
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
    while (!NodeIsLeaf(qTree->root)){
        InternalNodeAddRemoveLock( (InternalNode*)qTree->root);
        InternalNode *internalNode = (InternalNode*)qTree->root;
        if(internalNode->node.allocated == 0){
            qTree->root = internalNode->childs[0];
//            vmlog(MiXLog, "change root, rm node:%d, pointer:%lx", internalNode->node.id, internalNode);
            free((void *)internalNode);
        } else{
            InternalNodeRmRemoveLock( (InternalNode*)qTree->root);
            break;
        }
    }
    pthread_spin_unlock(&qTree->removeLock);
//    NodeCheckTree(qTree->root);
}


BOOL QTreeCheckMaxMin(QTree* qTree){
    return NodeCheckMaxMin(qTree->root);
}

BOOL QTreeCheckKey(QTree* qTree){
    return NodeCheckKey(qTree->root);
}