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
    qTree->root->allowSplit = 1;
    qTree->elements = 0;
    qTree->height = 1;
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
    config_lookup_int(&cfg, "removePoint", (int*)&removePoint);
    config_lookup_int(&cfg, "searchKeyType", (int*)&searchKeyType);
    config_lookup_int(&cfg, "optimizationType", (int*)&optimizationType);
    config_lookup_int(&cfg, "checkQueryMeta", &checkQueryMeta);

    if(markDelete){
        pthread_create(&RefactorThread, 0, (void *(*)(void *))QTreeRefactorThread, (void *)qTree);
    }

}

void QTreeRefactorThread(QTree* qTree){
    while (WorkEnd == FALSE){
        int idealNum = 1;
        for (int i = 0; i < (qTree->height - 1); i ++){
            idealNum *= (Border / 2);
        }
        if(qTree->height > 1 && qTree->elements < idealNum){
            QTreeRefactor(qTree, MaxThread);
        } else{
            vmlog(WARN, "no need refactor, element:%d, height:%d", qTree->elements, qTree->height);
            usleep(5000000); // 5s
            continue;
        }
    }
    RefactorThreadEnd = TRUE;
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
    nodeRootNew->node.allowSplit = 1;
    qTree->height ++;
//    vmlog(RemoveLog, "QTreeMakeNewRoot :%d", nodeRootNew->node.id);
}

inline void setSearchKey(KeyType * key, int threadId){
//    node->tree->funcCount ++;
    switch (searchKeyType) {
        case LOW:
            key->searchKey = key->lower;
            break;
        case DYMID:
            key->searchKey = key->lower;
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
                int index = clockIndex[threadId];
                for (int i = 0; i < RemovedQueueSize; ++i) {
                    index --;
                    index = index < 0? index + RemovedQueueSize: index;
                    if(QueryRangeCover(*key, RemovedKey[threadId][index])){
                        key->searchKey = RemovedKey[threadId][index];
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
    if(node == NULL){
        return FALSE;
    }
    BOOL success;
    if(NodeIsLeaf(node)){
        success = NodeTryAddInsertWriteLock(node);
    } else{
        success = NodeTryAddInsertReadLock(node, threadId);
    }
    if(!NodeIsValid(node)){
        if(NodeIsLeaf(node)){
            NodeRmInsertWriteLock(node);
        } else{
            NodeRmInsertReadLock(node, threadId);
        }
        success = FALSE;
    }
    return success;
}

inline void QTreeRmLockForFindLeaf(Node* node, int threadId){
    if(node ==NULL){
        return;
    }
    if(NodeIsLeaf(node)){
        NodeRmInsertWriteLock(node);
    } else{
        NodeRmInsertReadLock(node, threadId);
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
        //        vmlog(WARN, "QTreeTravelRightLink, key:%d, node:%d, nextNodeMin:%d, right:%d",
        //              key->searchKey, node->id, node->nextNodeMin, node->right->id);
        //        printf("QTreeTravelRightLink, key:%d, node:%d, nextNodeMin:%d, right:%d\n",
        //              key->searchKey, node->id, node->nextNodeMin, node->right->id);
        if(QTreeAddLockForFindLeaf(node->right, threadId) == FALSE){
            QTreeRmLockForFindLeaf(node, threadId);
            return NULL;
        }
        Node* temp = node;
        node =  node->right;
        QTreeRmLockForFindLeaf(temp, threadId);
    }
    return node;
}

inline LeafNode* QTreeFindLeafNode(QTree* qTree, KeyType * key, NodesStack* nodesStack, int threadId) {
    qTree->funcCount ++;
    Node* node  = NULL;
    int findTime = 0;
    findAgain:{
        findTime ++;
        if(findTime % 1000 == 0){
            if(findTime % 10000 == 0){
                vmlog(WARN,"QTreeFindLeafNode retry:%d", findTime);
            }
            if(findTime % 1000000 == 0){
                while (!stackEmpty(nodesStack->stackNodes, nodesStack->stackNodesIndex)){
                    node = (Node*)stackPop(nodesStack->stackNodes, nodesStack->stackNodesIndex);
                    vmlog(WARN, "stackNode: %d", node->id);
                }
                vmlog(ERROR,"QTreeFindLeafNode retry:%d", findTime);
            }
            usleep(1000);
        }
        while (!stackEmpty(nodesStack->stackNodes, nodesStack->stackNodesIndex)){
            node = (Node*)stackPop(nodesStack->stackNodes, nodesStack->stackNodesIndex);
            QTreeRmLockForFindLeaf(node,threadId);
        }
        node = qTree->root;
        int slot = 0;
        if(QTreeAddLockForFindLeaf(node, threadId) == FALSE){
            goto  findAgain;
        }
        //no need travel left
        node = QTreeTravelRightLink(node, key, threadId);
        if(node == NULL){
            goto findAgain;
        }
        while (!NodeIsLeaf(node)) {
            InternalNode *nodeInternal = (InternalNode*) node;
            if(nodeInternal->node.left != NULL && key->searchKey < nodeInternal->node.left->nextNodeMin){
                printQueryRange(key);
                printf("slot:%d\n", slot);
                printInternalNode(nodeInternal);
                printInternalNode((InternalNode*)nodeInternal->node.left);
                while (!stackEmpty(nodesStack->stackNodes, nodesStack->stackNodesIndex)){
                    InternalNode* node = stackPop(nodesStack->stackNodes, nodesStack->stackNodesIndex);
                    printInternalNode(node);
                }
                exit(-1);
                vmlog(ERROR, "QTreeFindLeafNode: error slot:%d\n", slot);
            }
            stackPush(nodesStack->stackNodes, nodesStack->stackNodesIndex, nodeInternal);
            slot = InternalNodeFindSlotByKey(nodeInternal, key);
            slot = ((slot < 0) ? (-slot) - 1 : slot + 1);
            node =nodeInternal->childs[slot];
            QTreeRmLockForFindLeaf((Node*)nodeInternal, threadId);
            if(QTreeAddLockForFindLeaf(node, threadId) == FALSE){
                goto  findAgain;
            }
            node = QTreeTravelRightLink(node, key, threadId);
            if(node == NULL || !NodeIsValid(node)){
                QTreeRmLockForFindLeaf(node, threadId);
                goto findAgain;
            }
//            vmlog(WARN, "QTreeFindLeafNode, key:%d, internalNode:%d, allocated:%d, slot:%d, child:%d",
//                  key->searchKey, nodeInternal->node.id, nodeInternal->node.allocated, slot, node->id);

        }

    }

    return (NodeIsLeaf(node) ? (LeafNode*) node : NULL);
}

void QTreePut(QTree* qTree, QueryMeta * value, int threadId){
    if(value == NULL){
        return ;
    }
    switch (optimizationType) {
        case None:
        case NoSort:
            setSearchKey(&value->dataRegion, threadId);
            QTreePutOne(qTree, &value->dataRegion, value, threadId);
            return;
    }
    // empty batch
    if( qTree->batchCount[threadId] == 0){
        setSearchKey(&value->dataRegion, threadId);
        qTree->batchSearchKey[threadId] = value->dataRegion.searchKey;
        qTree->batch[threadId][0].key = value->dataRegion;
        qTree->batch[threadId][0].value = value;
        qTree->batchCount[threadId] ++;
        qTree->batchMissCount[threadId] = 0;
        return;
    }

    // put into the batch
    if( qTree->batchCount[threadId] > 0 && QueryRangeCover(value->dataRegion, qTree->batchSearchKey[threadId]) && qTree->batchCount[threadId] < MaxBatchCount){
        value->dataRegion.searchKey = qTree->batchSearchKey[threadId];
        int innerIndex = qTree->batchCount[threadId];
        qTree->batch[threadId][innerIndex].key = value->dataRegion;
        qTree->batch[threadId][innerIndex].value = value;
        qTree->batchCount[threadId] ++;
        if(qTree->batchCount[threadId] >= MaxBatchCount){
            QTreePutBatch(qTree, qTree->batch[threadId], qTree->batchCount[threadId], threadId);
            qTree->batchCount[threadId] = 0;
        }
        qTree->batchMissCount[threadId] = 0;
        return;
    }

    // replace the  batch or insert the key directly
    if(qTree->batchMissCount[threadId] > batchMissThreshold){
        QTreePutBatch(qTree, qTree->batch[threadId], qTree->batchCount[threadId], threadId);
        setSearchKey(&value->dataRegion, threadId);
        qTree->batchSearchKey[threadId] = value->dataRegion.searchKey;
        qTree->batch[threadId][0].key = value->dataRegion;
        qTree->batch[threadId][0].value = value;
        qTree->batchCount[threadId] = 1;
        qTree->batchMissCount[threadId] = 0;
    } else{
        setSearchKey(&value->dataRegion, threadId);
        QTreePutOne(qTree, &value->dataRegion, value, threadId);
        qTree->batchMissCount[threadId] ++;
    }

}

inline void QTreePutOne(QTree* qTree, QueryRange* key, QueryMeta* value, int threadId){
    if(key == NULL || value == NULL){
        return ;
    }

    NodesStack  nodesStack;
    nodesStack.stackNodesIndex = 0;
    BoundKey min = key->lower, max = key->upper;
    LeafNode* nodeLeaf;
    int slot;
    nodeLeaf = QTreeFindLeafNode(qTree, key, &nodesStack, threadId);
    if(nodeLeaf->node.left != NULL && key->searchKey < nodeLeaf->node.left->nextNodeMin){
        printQueryRange(key);
        printLeafNode(nodeLeaf);
        printLeafNode((LeafNode*)nodeLeaf->node.left);
        while (!stackEmpty(nodesStack.stackNodes, nodesStack.stackNodesIndex)){
            InternalNode* node = stackPop(nodesStack.stackNodes, nodesStack.stackNodesIndex);
            printInternalNode(node);
        }
        vmlog(ERROR, "QTreePutOne: error \n");
    }
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
            vmlog(ERROR, "QTreePutOne: unSupport type:%d\n", optimizationType);
    }

    //

    Node*   splitedNode = (NodeIsFull((Node*)nodeLeaf) ? LeafNodeSplit(nodeLeaf) : NULL);

    QTreePropagateSplit( qTree, &nodesStack, nodeLeaf, splitedNode, restMaxMin, key->searchKey, min,  max,  threadId);
    qTree->elements ++;

    //    NodeCheckTree(qTree->root);
}

inline void QTreePropagateSplit(QTree* qTree, NodesStack* nodesStack, LeafNode* nodeLeaf, Node* splitedNode, BOOL restMaxMin, BoundKey key,  BoundKey min, BoundKey max, int threadId){
    Node*   lastNode = (Node*)nodeLeaf;
    if(splitedNode == NULL && !restMaxMin){
        NodeRmInsertWriteLock(lastNode);
    }
    int slot;
    // Iterate back over nodes checking overflow / splitting
    while (!stackEmpty(nodesStack.stackNodes, nodesStack->stackNodesIndex)) {
        //        cout << slot << endl;
        InternalNode* node = stackPop(nodesStack->stackNodes, nodesStack->stackNodesIndex);
        if(!NodeIsValid((Node*)node)){
            vmlog(WARN, "node invalid:%d", node->node.id);
        }
        //        vmlog(InsertLog, "QTreePutOne, stackPop node:%d", node->node.id);

        if (splitedNode != NULL) {
            NodeAddInsertWriteLock((Node*)node);
            // split occurred in previous phase, splitedNode is new child
            KeyType  childKey = NodeSplitShiftKeysLeft(splitedNode);
            while ((slot = InternalNodeFindSlotByChild(node, lastNode)) < 0){
//                vmlog(InsertLog, " node :%d not contain the child:%d", node->node.id, lastNode->id);
                BOOL leftNode;
                if(!NodeIsValid((Node*)node)){
                    vmlog(WARN, "node invalid:%d", node->node.id);
                    leftNode = TRUE;
                } else if(key >= node->node.nextNodeMin){
                    leftNode = FALSE;
                } else if(key < node->node.nextNodeMin){
                    vmlog(ERROR, "travel link ERROR: node :%d and its right not contain the key:%d", node->node.id, max);
                }
                if(leftNode == TRUE){
                    if(node->node.left == NULL){
                        vmlog(ERROR, "travel link ERROR: node :%d and has no left node", node->node.id);
                    }
                    NodeRmInsertWriteLock((Node*)node);
                    NodeAddInsertWriteLock(node->node.left);
                    node = (InternalNode*) node->node.left;
                } else{
                    if(node->node.right == NULL){
                        vmlog(ERROR, "travel link ERROR: node :%d and has no right node", node->node.id);
                    }
                    NodeAddInsertWriteLock(node->node.right);
                    Node* tempNode = (Node*)node;
                    node = (InternalNode*) node->node.right;
                    NodeRmInsertWriteLock(tempNode);
                }
            }
//            vmlog(InsertLog, "QTreePropagateSplit internalNode:%d add child:%d, slot:%d",
//                  node->node.id, splitedNode->id, slot);
            InternalNodeAdd(node, slot, &childKey, splitedNode);
            min = splitedNode->minValue < min? splitedNode->minValue: min;
            max = splitedNode->maxValue > max? splitedNode->maxValue: max;
            min = lastNode->minValue < min? lastNode->minValue: min;
            max = lastNode->maxValue > max? lastNode->maxValue: max;
            restMaxMin = QTreeModifyNodeMaxMin( (Node*)node, min, max);
            NodeRmInsertWriteLock(lastNode);
            NodeRmInsertWriteLock(splitedNode);
            if(NodeIsFull((Node*)node)){
                splitedNode = InternalNodeSplit(node);
            }else if( !restMaxMin){
                NodeRmInsertWriteLock((Node*)node);
                splitedNode = NULL;
            }
        } else if(restMaxMin){
            NodeAddInsertReadLock((Node*)node, threadId);
            while ((InternalNodeFindSlotByChildWithRight(node, lastNode)) < 0){
                BOOL leftNode;
                if(!NodeIsValid((Node*)node)){
                    vmlog(WARN, "node invalid:%d", node->node.id);
                    leftNode = TRUE;
                } else if(key >= node->node.nextNodeMin){
                    leftNode = FALSE;
                } else if(key < node->node.nextNodeMin){
                    vmlog(ERROR, "travel link ERROR: node :%d and its right not contain the key:%d", node->node.id, max);
                }
                if(leftNode == TRUE){
                    if(node->node.left == NULL){
                        vmlog(WARN, "travel link ERROR: node :%d and has no left node", node->node.id);
                        NodeRmInsertReadLock(lastNode, threadId);
                        NodeRmInsertReadLock((Node*)node, threadId);
                        restMaxMin = FALSE;
                        break;
                    }
                    NodeRmInsertReadLock((Node*)node, threadId);
                    NodeAddInsertReadLock(node->node.left, threadId);
                    node = (InternalNode*) node->node.left;
                } else{
                    NodeAddInsertReadLock(node->node.right, threadId);
                    Node* tempNode = (Node*)node;
                    node = (InternalNode*) node->node.right;
                    NodeRmInsertReadLock(tempNode, threadId);
                }

            }
            if(restMaxMin){
                restMaxMin = QTreeModifyNodeMaxMin( (Node*)node, min, max);
                NodeRmInsertReadLock(lastNode, threadId);
                if(!restMaxMin){
                    NodeRmInsertReadLock((Node*)node, threadId);
                }
            }
        }
        lastNode = (Node*) node;
        //    splitedNode = (node->isFull() ? node->split() : NULL);
    }

    //handle root
    if (splitedNode != NULL) {   // root was split, make new root
        if(lastNode ==  qTree->root){
            QTreeMakeNewRoot(qTree, splitedNode);
            NodeRmInsertWriteLock(splitedNode);
        } else{
            while (lastNode != qTree->root){
                InternalNode* node = (InternalNode*) qTree->root;
                NodeAddInsertWriteLock((Node*)node);
                KeyType  childKey = NodeSplitShiftKeysLeft(splitedNode);
                while ((slot = InternalNodeFindSlotByChild(node, lastNode)) < 0){
                    if(node ->node.right == NULL){
                        vmlog(WARN, "ERROR: root split: miss max: %d, min:%d", max, min);
                        NodeAddInsertWriteLock((Node*)node);
                        break;
                    }
                    NodeAddInsertWriteLock(node->node.right);
                    Node* tempNode = (Node*)node;
                    node = (InternalNode*) node->node.right;
                    NodeRmInsertWriteLock(tempNode);
                }
                if(splitedNode != NULL){
                    InternalNodeAdd(node, slot, &childKey, splitedNode);
                    NodeRmInsertWriteLock(splitedNode);
                }
                if(restMaxMin){
                    restMaxMin = QTreeModifyNodeMaxMin( (Node*)node, min, max);
                }
                NodeRmInsertWriteLock(lastNode);
                splitedNode = NULL;
                lastNode = (Node*) node;
            }
        }
        NodeRmInsertWriteLock(lastNode);
    } else if(lastNode != qTree->root && restMaxMin){
        while (lastNode != qTree->root && restMaxMin){
            InternalNode* node = (InternalNode*) qTree->root;
            NodeAddInsertReadLock((Node*)node, threadId);

            while ((InternalNodeFindSlotByChildWithRight(node, lastNode)) < 0){
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
}

inline void QTreePutBatch(QTree* qTree, QueryData * batch, int batchCount, int threadId){
    if(batchCount == 0){
        return ;
    }

    NodesStack  nodesStack;
    nodesStack.stackNodesIndex = 0;
    BoundKey min = batch[0].key.lower, max = batch[0].key.upper;

    LeafNode* nodeLeaf = QTreeFindLeafNode(qTree, &batch[0].key, &nodesStack, threadId);
    if (nodeLeaf == NULL) {
        vmlog(ERROR, "QTreeFindLeafNode error!\n");
    }

    Node*   splitedNode ;
    int slot;
    BOOL restMaxMin = FALSE;
    switch (optimizationType) {
        case BatchAndNoSort:{
            int inserted = (batchCount > (Border - nodeLeaf->node.allocated))? (Border - nodeLeaf->node.allocated) : batchCount;
            restMaxMin = LeafNodeAddLastBatch(nodeLeaf, batch, inserted, &min, &max);
            splitedNode = (NodeIsFull((Node*)nodeLeaf) ? LeafNodeSplit(nodeLeaf) : NULL);
            LeafNode* insertToNode;
            while (inserted < batchCount){
                if(splitedNode != NULL ){ // something remains in the batch
                    if(QueryRangeLT(batch[inserted].key, (((LeafNode* )splitedNode)->data[0].key))){
                        insertToNode = nodeLeaf;
                    } else{
                        insertToNode = (LeafNode* )splitedNode;
                    }
                } else{
                    insertToNode = nodeLeaf;
                }
                int insertBatch = ((batchCount - inserted) > (Border - insertToNode->node.allocated))? (Border - insertToNode->node.allocated) : (batchCount - inserted);
                restMaxMin = LeafNodeAddLastBatch(insertToNode, batch + inserted, insertBatch, &min, &max) || restMaxMin;
                inserted += insertBatch;
                if(splitedNode == NULL){
                    splitedNode = (NodeIsFull((Node*)insertToNode) ? LeafNodeSplit(insertToNode) : NULL);
                }
            }

            break;
        }
        case Batch:{
            // Find in leaf node for key
            slot = LeafNodeFindSlotByKey(nodeLeaf, &batch[0].key);
            slot = (slot >= 0)?(slot + 1):((-slot) - 1);
            int inserted = nodeLeaf->node.allocated + batchCount <= Border? batchCount : (Border - nodeLeaf->node.allocated);
            restMaxMin = LeafNodeAddBatch(nodeLeaf, slot, batch, inserted, &min, &max);
            splitedNode = (NodeIsFull((Node*)nodeLeaf) ? LeafNodeSplit(nodeLeaf) : NULL);
            LeafNode* insertToNode;
            while (inserted < batchCount){
                if(splitedNode != NULL ){ // something remains in the batch
                    if(QueryRangeLT(batch[inserted].key, (((LeafNode* )splitedNode)->data[0].key))){
                        insertToNode = nodeLeaf;
                    } else{
                        insertToNode = (LeafNode* )splitedNode;
                    }
                } else{
                    insertToNode = nodeLeaf;
                }
                int insertBatch = ((batchCount - inserted) > (Border - insertToNode->node.allocated))? (Border - insertToNode->node.allocated) : (batchCount - inserted);
                slot = LeafNodeFindSlotByKey(insertToNode, &batch[0].key);
                slot = (slot >= 0)?(slot + 1):((-slot) - 1);
                restMaxMin = LeafNodeAddBatch(insertToNode, slot, batch + inserted, insertBatch, &min, &max) || restMaxMin;
                inserted += insertBatch;
                if(splitedNode == NULL){
                    splitedNode = (NodeIsFull((Node*)insertToNode) ? LeafNodeSplit(insertToNode) : NULL);
                }
            }
            break;
        }
        default:
            vmlog(ERROR, "QTreePutBatch: unSupport type:%d\n", optimizationType);
    }
    QTreePropagateSplit( qTree, &nodesStack, nodeLeaf, splitedNode, restMaxMin, batch[0].key.searchKey, min,  max,  threadId);

    qTree->elements += batchCount;
}



//delete queries in the batch queue
inline void QTreeCheckBatch(QTree* qTree, int attribute, Arraylist* removedQuery){
    int newCount = 0;
    for (int j = 0; j < qTree->batchCount[0]; ++j) {
        if(QueryRangeCover(qTree->batch[0][j].key, attribute)){
            ArraylistAdd(removedQuery, qTree->batch[0][j].value);
        } else{
            qTree->batch[0][newCount ++] = qTree->batch[0][j];
        }
    }
    qTree->batchCount[0] = newCount;
}

inline Node* checkInternalNode(QTree* qTree, InternalNode* nodeInternal,  KeyType* key, NodesStack *nodesStack, IntStack* slotStack, int threadId){
    checkInternal ++;
    Node* node = NULL;
    NodeAddInsertReadLock((Node*)nodeInternal, threadId);
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
    NodeRmInsertReadLock((Node*)nodeInternal, threadId);

    return node;
}

inline BOOL CheckLeafNodeCover(LeafNode * leafNode, int i,  BoundKey attribute){
    if(checkQueryMeta){
        return QueryMetaCover(((LeafNode*)leafNode)->data[i].value, attribute);
    }else{
        return  QueryRangeCover (((LeafNode*)leafNode)->data[i].key, attribute);
    }
}

inline void checkLeafNode(QTree* qTree, LeafNode* leafNode, BoundKey* removedMax, BoundKey* removedMin, BoundKey attribute, Arraylist* removedQuery, int threadId){
    int j = 0;
    BOOL resetMax = FALSE;
    BOOL resetMin = FALSE;
    NodeAddInsertWriteLock((Node*)leafNode);
    for(int i = 0; i < leafNode->node.allocated ; i ++){
        checkQuery ++;
        BOOL delete = FALSE;
        if(markDelete){
            if(QueryIsDeleted(leafNode->data[i].value)){
                delete = TRUE;
            }
        }
        if((delete == FALSE) && CheckLeafNodeCover(leafNode,i, attribute)){
            delete = TRUE;
            QuerySetDeleteFlag(leafNode->data[i].value);
            ArraylistAdd(removedQuery, leafNode->data[i].value);
            qTree->elements --;
        }
        if(delete == TRUE){
            if(leafNode->data[i].key.upper >= leafNode->node.maxValue){
                resetMax = TRUE;
            }
            if(leafNode->data[i].key.lower <= leafNode->node.minValue){
                resetMin = TRUE;
            }

        }else {
            leafNode->data[j ++] = leafNode->data[i];
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
    NodeRmInsertWriteLock((Node*)leafNode);
}

inline Node* getAnotherNode(QTree* qTree, KeyType* key, BoundKey* removedMax, BoundKey* removedMin, Arraylist* removedQuery,
                            NodesStack *nodesStack, IntStack* slotStack, int threadId){
    Node* node = NULL;
    int slot;

    while (!stackEmpty(nodesStack->stackNodes, nodesStack->stackNodesIndex)){
        node = (Node*)stackPop(nodesStack->stackNodes, nodesStack->stackNodesIndex);
        slot = stackPop(slotStack->stackSlots, slotStack->stackSlotsIndex);

        InternalNode* internalNode = (InternalNode*) node;
        //the node has been added r-readLock
        //   NodeAddRemoveReadLock((Node*)internalNode, threadId);
        NodeAddInsertReadLock((Node*)internalNode, threadId);
        for (; slot < node->allocated; slot ++) {
//            NodeAddRWLock(internalNode->childs[slot]);
            if(((internalNode->childs[slot + 1]->maxValue) >= key->lower) && ((internalNode->childs[slot + 1]->minValue) <= key->upper)){
                node = internalNode->childs[slot + 1];
                stackPush(nodesStack->stackNodes, nodesStack->stackNodesIndex, internalNode);
                stackPush(slotStack->stackSlots, slotStack->stackSlotsIndex, slot + 1);
//                NodeRmRWLock(internalNode->childs[slot]);
//                NodeRmRemoveReadLock((Node*)internalNode, threadId);
                NodeRmInsertReadLock((Node*)internalNode, threadId);
                return node;
            }
//            NodeRmRWLock(internalNode->childs[slot]);
        }
        BOOL childMerge = FALSE;
        for (int i = 0; i <= node->allocated ; ++i) {
            if(NodeIsUnderFlow(internalNode->childs[i])){
                childMerge = TRUE;
//                break;
            }
        }

        NodeRmInsertReadLock((Node*)internalNode, threadId);
        if(childMerge){
            if ((NodeTryAddInsertWriteLockForRemove((Node*)internalNode)) == FALSE){
                // another thread is doing merge
                childMerge = FALSE;
            }
        }
        if(childMerge == FALSE){
            node = NULL;
            continue;
        }
        childMerge = FALSE;
        for(int i = 0; i <= node->allocated; i ++ ){
            if(i <= node->allocated){
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
        NodeRmInsertWriteLockForRemove((Node*)internalNode);
        node = NULL;
    }

    return node;
}

void QTreeFindAndRemoveRelatedQueries(QTree* qTree, int attribute, Arraylist* removedQuery, int threadId){
    if(searchKeyType == REMOVE){
        for (int i = 0; i < RemovedQueueSize; ++i) {
            clockIndex[threadId] = (clockIndex[threadId] + 1) % RemovedQueueSize;
            if(RemovedKey[threadId][clockIndex[threadId]] == attribute){
                break;
            }
            if(clockFlag[threadId] & (1 << clockIndex[threadId])){
                clockFlag[threadId] &= ~(1 << clockIndex[threadId]);
            } else{
                RemovedKey[threadId][clockIndex[threadId]] = attribute;
                clockFlag[threadId] |= (1 << clockIndex[threadId]);
                break;
            }
        }
    }
//    if(checkInternal > 20){
//        printf("checkInternal\n");
//    }
    NodesStack  nodesStack;
    IntStack slotStack;
    nodesStack.stackNodesIndex = 0;
    slotStack.stackSlotsIndex = 0;
    int oldCheckInternal = checkInternal, oldRemove = removedQuery->size;
    checkInternal = 0;
//    if(optimizationType == BatchAndNoSort || optimizationType == Batch){
//        QTreeCheckBatch(qTree, attribute, removedQuery);
//    }
    Node* node = qTree->root;
    KeyType  queryRange;
    KeyType* key = &queryRange;
    QueryRangeConstructorWithPara(key, attribute, attribute, TRUE, TRUE);
    BoundKey removedMax = 0, removedMin = maxValue << 1;
    while (TRUE) {
        while (!NodeIsLeaf(node)){
            InternalNode* nodeInternal = (InternalNode*) node;
            node = checkInternalNode( qTree, nodeInternal,   key, &nodesStack, &slotStack, threadId);
            //                System.out.println("getNode:" + getNode + ", node:" + node);
            if(node == NULL){
                node = getAnotherNode(qTree, key, &removedMax, &removedMin, removedQuery, &nodesStack, &slotStack, threadId);
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
            checkLeafNode(qTree, leafNode,  &removedMax, &removedMin, attribute, removedQuery, threadId);
            if(stackEmpty(nodesStack.stackNodes, nodesStack.stackNodesIndex)){
                break;
            }
            node = getAnotherNode(qTree, key, &removedMax, &removedMin, removedQuery, &nodesStack, &slotStack, threadId);
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
    int loop = 0;
    while (!NodeIsLeaf(qTree->root) && ((loop ++) < 10)){
        InternalNode *internalNode = (InternalNode*)qTree->root;
        if((internalNode->node.allocated == 0) && NodeTryAddInsertWriteLock((Node*)internalNode) ){
            if((internalNode->node.allocated == 0) && (qTree->root == (Node*)internalNode) && !NodeIsLeaf(qTree->root)){
                qTree->height --;
                qTree->root = internalNode->childs[0];
                vmlog(WARN, "change root, rm node:%d, pointer:%lx, new root:%d", internalNode->node.id, internalNode, qTree->root->id);
                internalNode->node.allocated = -1;
            }
            NodeRmInsertWriteLock((Node*)internalNode);
        } else{
            break;
        }
    }
//    NodeCheckTree(qTree->root);
}

inline Node* checkInternalNodeForRefactor(QTree* qTree, InternalNode* nodeInternal, NodesStack *nodesStack, IntStack* slotStack, int threadId){
    Node* node = NULL;
    if(nodeInternal->node.allocated >= 0){
        node = nodeInternal->childs[0];
        stackPush(nodesStack->stackNodes, nodesStack->stackNodesIndex, nodeInternal);
        stackPush(slotStack->stackSlots, slotStack->stackSlotsIndex, 0);
    }
    return node;
}

inline Node* getAnotherNodeForRefactor(QTree* qTree, BoundKey* removedMax, BoundKey* removedMin,
                                       NodesStack *nodesStack, IntStack* slotStack, int threadId){
    Node* node = NULL;
    int slot;

    while (!stackEmpty(nodesStack->stackNodes, nodesStack->stackNodesIndex)){
        node = (Node*)stackPop(nodesStack->stackNodes, nodesStack->stackNodesIndex);
        slot = stackPop(slotStack->stackSlots, slotStack->stackSlotsIndex);

        InternalNode* internalNode = (InternalNode*) node;
        NodeAddInsertReadLock(node, threadId);
        if(slot < node->allocated){
            node = internalNode->childs[slot + 1];
            stackPush(nodesStack->stackNodes, nodesStack->stackNodesIndex, internalNode);
            stackPush(slotStack->stackSlots, slotStack->stackSlotsIndex, slot + 1);
            NodeRmInsertReadLock((Node*)internalNode, threadId);
            return node;
        }
        BOOL childMerge = FALSE;
        for (int i = 0; i <= node->allocated ; ++i) {
            if(NodeIsUnderFlow(internalNode->childs[i])){
                childMerge = TRUE;
                break;
            }
        }
        NodeRmInsertReadLock((Node*)internalNode, threadId);
        if(childMerge){
            if ((NodeTryAddInsertWriteLockForRemove((Node*)internalNode)) == FALSE){
                // another thread is doing merge
                childMerge = FALSE;
            }
        }
        if(childMerge == FALSE){
            node = NULL;
            continue;
        }
        childMerge = FALSE;
        for(int i = 0; i <= node->allocated; i ++ ){
            childMerge = InternalNodeCheckUnderflowWithRight(((InternalNode*) node), i) || childMerge;
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
        NodeRmInsertWriteLockForRemove((Node*)internalNode);
        node = NULL;
    }

    return node;
}



inline void checkLeafNodeForRefactor(QTree* qTree, LeafNode* leafNode, BoundKey* removedMax, BoundKey* removedMin, int threadId){
    int j = 0;
    BOOL resetMax = FALSE;
    BOOL resetMin = FALSE;
    NodeAddInsertWriteLock((Node*)leafNode);
    for(int i = 0; i < leafNode->node.allocated ; i ++){
        if(QueryIsDeleted(leafNode->data[i].value)){
            //            free(leafNode->data[i].value);
            if(leafNode->data[i].key.upper >= leafNode->node.maxValue){
                resetMax = TRUE;
            }
            if(leafNode->data[i].key.lower <= leafNode->node.minValue){
                resetMin = TRUE;
            }
            continue;
        }else {
            leafNode->data[j ++] = leafNode->data[i];
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
    NodeRmInsertWriteLock((Node*)leafNode);
}

void QTreeRefactor(QTree* qTree, int threadId){
    vmlog(WARN, "QTreeRefactor, element:%d, height:%d", qTree->elements, qTree->height);
    NodesStack  nodesStack;
    IntStack slotStack;
    nodesStack.stackNodesIndex = 0;
    slotStack.stackSlotsIndex = 0;
    Node* node = qTree->root;
    BoundKey removedMax = 0, removedMin = maxValue;
    while (TRUE) {
        while (!NodeIsLeaf(node)){
            InternalNode* nodeInternal = (InternalNode*) node;
            node = checkInternalNodeForRefactor( qTree, nodeInternal, &nodesStack, &slotStack, threadId);
            //                System.out.println("getNode:" + getNode + ", node:" + node);
            if(node == NULL){
                node = getAnotherNodeForRefactor(qTree, &removedMax, &removedMin,  &nodesStack, &slotStack, threadId);
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
            checkLeafNodeForRefactor(qTree, leafNode,  &removedMax, &removedMin, threadId);
            if(stackEmpty(nodesStack.stackNodes, nodesStack.stackNodesIndex)){
                break;
            }
            node = getAnotherNodeForRefactor(qTree, &removedMax, &removedMin, &nodesStack, &slotStack, threadId);
            if(node == NULL){
                break;
            }
        }else {
            break;
        }
    }
    while (!NodeIsLeaf(qTree->root)){
        InternalNode *internalNode = (InternalNode*)qTree->root;

        if((internalNode->node.allocated == 0) && NodeTryAddInsertWriteLock((Node*)internalNode) ){
            if((internalNode->node.allocated == 0) && (qTree->root == (Node*)internalNode) && !NodeIsLeaf(qTree->root)){
                qTree->height --;
                qTree->root = internalNode->childs[0];
                vmlog(WARN, "change root, rm node:%d, pointer:%lx, new root:%d", internalNode->node.id, internalNode, qTree->root->id);
                internalNode->node.allocated = -1;
            }
            NodeRmInsertWriteLock((Node*)internalNode);
        } else{
            break;
        }
    }
}


BOOL QTreeMarkDelete(QTree* qTree, QueryMeta* queryMeta){
    if(!QueryIsDeleted(queryMeta)){
        QuerySetDeleteFlag(queryMeta);
        qTree->elements --;
        return TRUE;
    } else{
        return FALSE;
    }

}


inline Node* checkInternalNodeForDelete(QTree* qTree, InternalNode* nodeInternal,  KeyType* key, NodesStack *nodesStack, IntStack* slotStack, int threadId){
//    checkInternal ++;
    Node* node = NULL;
    NodeAddInsertReadLock((Node*)nodeInternal, threadId);
    int slot;
    while ((slot = InternalNodeFindMinSlotByKey( nodeInternal, key->searchKey)) < 0){
        if(nodeInternal->node.nextNodeMin > key->searchKey){
            NodeRmInsertReadLock((Node*)nodeInternal, threadId);
//            vmlog(RemoveLog, "checkInternalNodeForDelete not found key:%d", key->searchKey);
            return NULL;
        } else{
            InternalNode* temp = nodeInternal;
            NodeAddInsertReadLock(nodeInternal->node.right, threadId);
            nodeInternal = (InternalNode*)nodeInternal->node.right;
            NodeRmInsertReadLock((Node*)temp, threadId);
        }
    }
    node = nodeInternal->childs[slot];
    if(node ==NULL){
        vmlog(WARN, "checkInternalNodeForDelete, slot:%d, node:%d", slot, nodeInternal->node.id);
    }
    stackPush(nodesStack->stackNodes, nodesStack->stackNodesIndex, nodeInternal);
    stackPush(slotStack->stackSlots, slotStack->stackSlotsIndex, slot);
    NodeRmInsertReadLock((Node*)nodeInternal, threadId);
    return node;
}

inline LeafNode * checkLeafNodeForDelete(QTree* qTree, LeafNode* leafNode, QueryMeta * queryMeta, int threadId){

    BOOL find = FALSE;
    NodeAddInsertWriteLock((Node*)leafNode);
    int retry = 0;
    refindLeaf:{
        retry ++;
        if(retry > 1000000){
            vmlog(WARN, "checkLeafNodeForDelete retry:%d", retry);
        }
        int i;
        for(i = 0; i < leafNode->node.allocated ; i ++){
            if(leafNode->data[i].value == queryMeta){
                find = TRUE;
                break;
            }
        }
        if(find){
//            free(leafNode->data[i].value);
            memcpy(leafNode->data + i, leafNode->data + i + 1, (leafNode->node.allocated - i - 1) * sizeof (QueryData));
            leafNode->node.allocated--;
            NodeRmInsertWriteLock((Node*)leafNode);
        } else if(leafNode->node.nextNodeMin > queryMeta->dataRegion.searchKey){
            NodeRmInsertWriteLock((Node*)leafNode);
//            vmlog(RemoveLog, "checkLeafNodeForDelete not found query:%s", queryMeta->queryId);
            return NULL;
        } else{
            LeafNode* temp = leafNode;
            NodeAddInsertWriteLock(leafNode->node.right);
            leafNode = (LeafNode*)leafNode->node.right;
            NodeRmInsertWriteLock((Node*)temp);
            goto refindLeaf;
        }
    }


    return leafNode;
}

inline void QTreePropagateMerge(QTree* qTree, Node* lastNode,  NodesStack *nodesStack, IntStack* slotStack, int threadId){
    Node* node = NULL;
    int slot;
    BOOL childMerge = NodeIsUnderFlow(lastNode);
    while (!stackEmpty(nodesStack->stackNodes, nodesStack->stackNodesIndex)){
        node = (Node*)stackPop(nodesStack->stackNodes, nodesStack->stackNodesIndex);

        //the node has been added r-readLock
        if(childMerge == FALSE){
            continue;
        }
        if (NodeTryAddInsertWriteLockForRemove(node) == FALSE){
            // another thread is doing merge
            childMerge = FALSE;
            lastNode = NULL;
            continue;
        }

        while ((slot = InternalNodeFindSlotByChild((InternalNode*) node, lastNode)) < 0){
            //                vmlog(InsertLog, " node :%d not contain the child:%d", node->node.id, lastNode->id);

            BOOL leftNode;
            if(!NodeIsValid((Node*)node)){
                vmlog(WARN, "node invalid:%d", node->id);
                leftNode = TRUE;
            } else if(node->nextNodeMin <= lastNode->nextNodeMin){
                leftNode = FALSE;
            } else if(node->nextNodeMin > lastNode->nextNodeMin){
                vmlog(WARN, "travel link ERROR: node :%d and its right not contain the child:%d", node->id, lastNode->id);
                childMerge = FALSE;
                NodeRmInsertWriteLockForRemove(node);
                break;
            }
            if(leftNode == TRUE){
                if(node->left == NULL){
                    vmlog(ERROR, "travel link ERROR: node :%d and has no left node", node->id);
                }
                NodeRmInsertWriteLockForRemove(node);
                if (NodeTryAddInsertWriteLockForRemove(node->left) == FALSE){
                    // another thread is doing merge
                    childMerge = FALSE;
                    break;
                } else{
                    node = node->left;
                }

            } else{
                if(node->right == NULL){
                    vmlog(ERROR, "travel link ERROR: node :%d and has no right node", node->id);
                }
                if (NodeTryAddInsertWriteLockForRemove(node->right) == FALSE){
                    // another thread is doing merge
                    childMerge = FALSE;
                    NodeRmInsertWriteLockForRemove(node);
                    break;
                } else{
                    Node* tempNode = (Node*)node;
                    node = node->right;
                    NodeRmInsertWriteLockForRemove(tempNode);
                }
            }
        }
        if(childMerge == FALSE){
            continue;
        }
        InternalNodeCheckUnderflowWithRight((InternalNode*) node, slot) ;
        childMerge = NodeIsUnderFlow(node);
        NodeRmInsertWriteLockForRemove(node);
        lastNode = node;
    }
}

BOOL QTreeDeleteQuery(QTree* qTree, QueryMeta * queryMeta, int threadId){
    NodesStack  nodesStack;
    IntStack slotStack;
    nodesStack.stackNodesIndex = 0;
    slotStack.stackSlotsIndex = 0;
    Node* node = qTree->root;
    KeyType* key = &queryMeta->dataRegion;
    BOOL found = FALSE;
    while (!NodeIsLeaf(node)){
        node = checkInternalNodeForDelete( qTree, (InternalNode*) node,   key, &nodesStack, &slotStack, threadId);
        if(node == NULL){
            while (!stackEmpty(nodesStack.stackNodes, nodesStack.stackNodesIndex)){
                node = (Node*)stackPop(nodesStack.stackNodes, nodesStack.stackNodesIndex);
            }
            return FALSE;
        }
    }
    if(NodeIsLeaf(node)){
        LeafNode* leafNode = (LeafNode*) node;
        //                System.out.println("getLeafNode:" + leafNode);
        leafNode = checkLeafNodeForDelete(qTree, leafNode,  queryMeta, threadId);
        if (leafNode == NULL){ // not found
            while (!stackEmpty(nodesStack.stackNodes, nodesStack.stackNodesIndex)){
                node = (Node*)stackPop(nodesStack.stackNodes, nodesStack.stackNodesIndex);
            }
        } else{
            found = TRUE;
            qTree->elements --;
            QTreePropagateMerge(qTree, (Node*)leafNode,  &nodesStack, &slotStack, threadId);
        }
    }
    int loop = 0;
    while (!NodeIsLeaf(qTree->root) && ((loop ++) < 10)){
        InternalNode *internalNode = (InternalNode*)qTree->root;

        if(internalNode->node.allocated == 0){
            if(NodeTryAddInsertWriteLock((Node*)internalNode) ){
                if((internalNode->node.allocated == 0) && (qTree->root == (Node*)internalNode) && !NodeIsLeaf(qTree->root)){
                    qTree->height --;
                    qTree->root = internalNode->childs[0];
                    vmlog(WARN, "change root, rm node:%d, pointer:%lx, new root:%d", internalNode->node.id, internalNode, qTree->root->id);
                    internalNode->node.allocated = -1;
                }
                NodeRmInsertWriteLock((Node*)internalNode);
            } else{
                break;
            }
        } else{
            break;
        }
    }
    return found;

}

BOOL QTreeCheckMaxMin(QTree* qTree){
    return NodeCheckMaxMin(qTree->root);
}

BOOL QTreeCheckKey(QTree* qTree){
    return NodeCheckKey(qTree->root);
}