//
// Created by workshop on 8/24/2021.
//
#include <Tool/ArrayList.h>
#include <time.h>
#include <libconfig.h>
#include "QTree.h"

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
    config_setting_t *setting;
    const char *str;

    config_init(&cfg);

    /* Read the file. If there is an error, report it and exit. */
    if(! config_read_file(&cfg, ConfigFile))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
    }
    int regionType, keyType;
    config_lookup_int(&cfg, "searchKeyType", &keyType);
    switch (keyType) {
        case 0:
            searchKeyType = LOW;
            break;
        case 1:
            searchKeyType = DYMID;
            break;
        case 2:
            searchKeyType = Mid;
            break;
        case 3:
            searchKeyType = RAND;
            break;
    }

//    printf("QTree searchKeyType = %d\n", searchKeyType);
//    memset(qTree->stackSlots,0, maxDepth * sizeof (int));
//    memset(qTree->stackNodes,0, maxDepth * sizeof (InternalNode *));
}

void QTreeDestroy(QTree* qTree){
//    printf("Border:%d, leafSplitCount: %d, internalSplitCount:%d, funcTime:%ld, funcCount:%d, whileCount:%d\n",
//           Border, qTree->leafSplitCount,  qTree->internalSplitCount,  qTree->funcTime,  qTree->funcCount,  qTree->whileCount);
    NodeDestroy(qTree->root);
}

void printQTree( QTree* qTree){
    int elements_debug_local_recounter = 0;
    Node* node = NULL;
    int depth = 0;
    int height = depth;
    Node* stackNodes[maxDepth];
    int stackNodesIndex = 0;
    int stackChildCount[maxDepth];
    int stackChildCountIndex = 0;
    stackPush(stackNodes,stackNodesIndex, qTree->root); // init seed, root node
    BOOL lastIsInternal = (BOOL)!qTree->root->isLeaf;
    while (!(stackEmpty(stackNodes, stackNodesIndex))) {
        node = stackPop(stackNodes, stackNodesIndex);
        int count = stackPop(stackChildCount, stackChildCountIndex);
        stackPush(stackChildCount, stackChildCountIndex, count - 1);
        if (!node->isLeaf) {
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
        lastIsInternal = (BOOL)!node->isLeaf;
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
extern SearchKeyType searchKeyType;

struct timespec startTmp, endTmp;

inline void setSearchKey(Node* node, KeyType * key){
    node->tree->funcCount ++;
    switch (searchKeyType) {
        case LOW:
            key->searchKey = key->lower;
            break;
        case DYMID:
            if((QueryRange*)node->maxValue == NULL){
                key->searchKey = (key->lower + key->upper) >> 1;
            } else if((key->lower < ((QueryRange*)node->maxValue)->upper) && (key-> upper > ((QueryRange*)node->minValue)->lower)){
                int low = ((QueryRange*)node->minValue)->lower;
                if(QueryRangeMinGT(key, (QueryRange*)node->minValue)){
                    low = key->lower;
                }
                int high =((QueryRange*)node->maxValue)->upper;
                if(!QueryRangeMaxGE(key, (QueryRange*)node->maxValue)){
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
    }
}

inline LeafNode* QTreeFindLeafNode(QTree* qTree, KeyType * key) {

    Node* node = qTree->root;
    int slot = 0;
    while (!node->isLeaf) {
        qTree->whileCount++;
        InternalNode *nodeInternal = (InternalNode*) node;

        setSearchKey(node, key);
//        clock_gettime(CLOCK_REALTIME, &startTmp);
        slot = NodeFindSlotByKey(node, key);
//        clock_gettime(CLOCK_REALTIME, &endTmp);
//        funcTime += (endTmp.tv_sec - startTmp.tv_sec) * 1e9 +  endTmp.tv_nsec - startTmp.tv_nsec;
        slot = ((slot < 0) ? (-slot) - 1 : slot + 1);

        stackPush(qTree->stackNodes, qTree->stackNodesIndex, nodeInternal);
        stackPush(qTree->stackSlots, qTree->stackSlotsIndex, slot);

        node =nodeInternal->childs[slot];
        if (node == NULL) {
            printf("ERROR childs[%d] in node %d \n", slot, nodeInternal->node.id);
            exit(-1);
        }
    }
    setSearchKey(node, key);

    return (node->isLeaf ? (LeafNode*) node : NULL);
}


Node* QTreePut(QTree* qTree, QueryRange * key, QueryMeta * value){
//    if(qTree->elements == 1519){
//        printNode(qTree->root);
//    }
    if(key == NULL || value == NULL){
        return NULL;
    }


    LeafNode* nodeLeaf = QTreeFindLeafNode(qTree, key);
    if (nodeLeaf == NULL) {
        printf("QTreeFindLeafNode error!\n");
        exit(-1);
    }
    //
    // Find in leaf node for key
    int slot = NodeFindSlotByKey((Node*)nodeLeaf, key);

    slot = (slot >= 0)?(slot + 1):((-slot) - 1);

    LeafNodeAdd(nodeLeaf, slot, key, value);
    Node*   splitedNode = (NodeIsFull((Node*)nodeLeaf) ? LeafNodeSplit(nodeLeaf) : NULL);
    // Iterate back over nodes checking overflow / splitting
    while (!stackEmpty(qTree->stackNodes, qTree->stackNodesIndex)) {
        //        cout << slot << endl;
        InternalNode* node = stackPop(qTree->stackNodes, qTree->stackNodesIndex);
        if(node->node.maxValue == NULL || QueryRangeMaxGE(key, node->node.maxValue) ){
            node->node.maxValue = key;
        }
        if(node->node.minValue == NULL || QueryRangeMinGT(node->node.minValue, key) ){
            node->node.minValue = key;
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



    qTree->elements++;
    if (splitedNode != NULL) {   // root was split, make new root
        QTreeMakeNewRoot(qTree, splitedNode);
    }
//    NodeCheckTree(qTree->root);
    return splitedNode;

}



void QTreeFindAndRemoveRelatedQueries(QTree* qTree, int attribute, Arraylist* removedQuery){
    Node* node = qTree->root;
    int slot = 0;
    KeyType  queryRange;
    KeyType* key = &queryRange;
    QueryRangeConstructorWithPara(key, attribute, attribute, TRUE, TRUE);
    QueryRange *removedMax, *removedMin;
    BOOL resetMax = FALSE;
    BOOL resetMin = FALSE;
    while (TRUE) {
        while (!node->isLeaf){
            BOOL getNode = FALSE;
            InternalNode* nodeInternal = (InternalNode*) node;
            for(slot = 0; slot <= nodeInternal->node.allocated; slot ++){
                if(QueryRangeMaxGE(nodeInternal->childs[slot]->maxValue, key)){
                    node = nodeInternal->childs[slot];
                    stackPush(qTree->stackNodes, qTree->stackNodesIndex, nodeInternal);
                    stackPush(qTree->stackSlots, qTree->stackSlotsIndex, slot);
                    getNode = TRUE;
                    break;
                }
                if(slot < nodeInternal->node.allocated && QueryRangeMinGT( nodeInternal->childs[slot + 1]->minValue, key)){
                    break;
                }
            }
            //                System.out.println("getNode:" + getNode + ", node:" + node);
            if(!getNode){
                BOOL getAnother = FALSE;
                while (!stackEmpty(qTree->stackNodes, qTree->stackNodesIndex)){
                    node = (Node*)stackPop(qTree->stackNodes, qTree->stackNodesIndex);
                    slot = stackPop(qTree->stackSlots, qTree->stackSlotsIndex);
                    if(slot >= node->allocated  || QueryRangeMinGT(((InternalNode*) node)->childs[slot + 1]->minValue, key)){
                        for(int i = 0; i < slot; i ++ ){
                            InternalNodeCheckUnderflowWithRight(((InternalNode*) node), i);
                        }
                        if(node->maxValue == removedMax){
                            InternalNodeResetMaxValue(((InternalNode*) node));
                        }
                        if(node->minValue == removedMin){
                            InternalNodeResetMinValue(((InternalNode*) node));
                        }
                    }else {
                        InternalNode* internalNode = (InternalNode*) node;
                        node = internalNode->childs[slot + 1];
                        stackPush(qTree->stackNodes, qTree->stackNodesIndex, internalNode);
                        stackPush(qTree->stackSlots, qTree->stackSlotsIndex, slot + 1);
                        //                            System.out.println("another node:" + node);
                        slot = 0;
                        getAnother = TRUE;
                        break;
                    }
                }
                if(!getAnother){
                    break;
                }
            }
        }
        if(node->isLeaf){
            int j = 0;
            LeafNode* leafNode = (LeafNode*) node;
            //                System.out.println("getLeafNode:" + leafNode);
            resetMax = FALSE;
            resetMin = FALSE;
            for(int i = 0; i < leafNode->node.allocated ; i ++){
                //                    System.out.println("query:" + leafNode.values[i]);
                if(QueryMetaCover(leafNode->values[i], attribute)){
                    if(leafNode->node.maxValue == &leafNode->node.keys[i]){
                        resetMax = TRUE;
                        removedMax = (leafNode->node.maxValue);
                    }
                    if(leafNode->node.minValue == &leafNode->node.keys[i]){
                        resetMin = TRUE;
                        removedMin = leafNode->node.minValue;
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
                LeafNodeResetMaxValue(leafNode);
            }else{
                removedMax = NULL;
            }
            if(resetMin){
                LeafNodeResetMinValue(leafNode);
            }else{
                removedMin = NULL;
            }
            if(stackEmpty(qTree->stackNodes, qTree->stackNodesIndex)){
                break;
            }
            BOOL getAnother = FALSE;
            while (!stackEmpty(qTree->stackNodes, qTree->stackNodesIndex)){
                node = (Node*)stackPop(qTree->stackNodes, qTree->stackNodesIndex);
                slot =stackPop(qTree->stackSlots, qTree->stackSlotsIndex);

                if(slot >= node->allocated  || QueryRangeMinGT(((InternalNode*) node)->childs[slot + 1]->minValue, key)){
                    for(int i = 0; i < slot; i ++ ){
                        InternalNodeCheckUnderflowWithRight(((InternalNode*) node),i);
                    }
                    if(node->maxValue == removedMax){
                        InternalNodeResetMaxValue(((InternalNode*) node));
                    }
                    if(node->minValue == removedMin){
                        InternalNodeResetMinValue(((InternalNode*) node));
                    }

                }else {
                    InternalNode* internalNode = (InternalNode*) node;
                    node = internalNode->childs[slot + 1];
                    stackPush(qTree->stackNodes, qTree->stackNodesIndex, internalNode);
                    stackPush(qTree->stackSlots, qTree->stackSlotsIndex, slot + 1);
                    slot = 0;
                    getAnother = TRUE;
                    break;
                }
            }
            if(!getAnother){
                break;
            }
        }else {
            break;
        }
    }
//    NodeCheckTree(qTree->root);
}


