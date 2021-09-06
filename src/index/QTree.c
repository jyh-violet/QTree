//
// Created by workshop on 8/24/2021.
//
#include <Tool/ArrayList.h>
#include <time.h>
#include "QTree.h"

void QTreeConstructor(QTree* qTree,  int BOrder){
    qTree->maxNodeID = 0;
    qTree->root = malloc(sizeof (LeafNode));
    LeafNodeConstructor((LeafNode*)qTree->root, qTree);
    LeafNodeAllocId((LeafNode*)qTree->root);
    qTree->elements = 0;
    qTree->stackNodesIndex = 0;
    qTree->stackSlotsIndex = 0;
    memset(qTree->stackSlots,0, maxDepth * sizeof (int));
    memset(qTree->stackNodes,0, maxDepth * sizeof (InternalNode *));
}

void QTreeDestroy(QTree* qTree){
    NodeDestroy(qTree->root);
    free(qTree->root);
}

void printQTree( QTree* qTree){
    int elements_debug_local_recounter = 0;
    Node* node = NULL;
    int depth = 0;
    int height = depth;
    Node* stackNodes[maxDepth];
    int stackNodesIndex = 0;
    push(stackNodes,stackNodesIndex, qTree->root); // init seed, root node
    bool lastIsInternal = !qTree->root->isLeaf;
    while (!(empty(stackNodes, stackNodesIndex))) {
        node = pop(stackNodes, stackNodesIndex);

        if (!node->isLeaf) {
            InternalNode* internalNode = (InternalNode*) node;
            for(int i = node->allocated; i >= 0; i --){
                push(stackNodes, stackNodesIndex, internalNode->childs[i]);
            }

        } else {
            elements_debug_local_recounter += node->allocated;
        }
        // For Indentation
        if (lastIsInternal || !node->isLeaf) { // Last or Curret are Internal
            depth += (lastIsInternal ? +1 : -1);
        }
        lastIsInternal = !node->isLeaf;
        int pad =(depth - 1);
        for (int i =0 ; i < pad; i ++){
            printf(" ");
        }

        printNode(node);
    }
    printf("height=%d root=%d elements=%d recounter=%d\n", height, qTree->root->id, qTree->elements, elements_debug_local_recounter);
}

int QTreeAllocNode(QTree* qTree, bool isLeaf){
    int id = qTree->maxNodeID ++;
    return isLeaf ? id : -id;
}

inline void QTreeMakeNewRoot(QTree* qTree, Node* splitedNode){
    InternalNode* nodeRootNew = malloc(sizeof (InternalNode));
    InternalNodeConstructor(nodeRootNew, qTree);
    InternalNodeAllocId(nodeRootNew);

    KeyType * newkey = NodeSplitShiftKeysLeft(splitedNode);
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
    funcCount ++;
    switch (searchKeyType) {
        case LOW:
            key->searchKey = key->lower;
            break;
            case DYMID:
                if((QueryRange*)node->maxValue == NULL){
                    key->searchKey = key->lower;
                } else if((key->lower < ((QueryRange*)node->maxValue)->upper) && (key-> upper > ((QueryRange*)node->minValue)->lower)){
                    int low = ((QueryRange*)node->minValue)->lower;
                    if(QueryRangeMinGT(key, (QueryRange*)node->minValue)){
                        low = key->lower;
                    }
                    int high =((QueryRange*)node->maxValue)->upper;
                    if(!QueryRangeMaxGE(key, (QueryRange*)node->maxValue)){
                        high = key->upper;
                    }
                    key->searchKey = (low + high) << 1;
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
    }
}

inline LeafNode* QTreeFindLeafNode(QTree* qTree, KeyType * key) {
    Node* node = qTree->root;
    int slot = 0;
    while (!node->isLeaf) {
        InternalNode *nodeInternal = (InternalNode*) node;

        setSearchKey(node, key);
        slot = NodeFindSlotByKey(node, key);
        slot = ((slot < 0) ? (-slot) - 1 : slot + 1);

        push(qTree->stackNodes, qTree->stackNodesIndex, nodeInternal);
        push(qTree->stackSlots, qTree->stackSlotsIndex, slot);

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
    int slot = NodeFindSlotByKey(nodeLeaf, key);

    slot = (slot >= 0)?(slot + 1):((-slot) - 1);

    LeafNodeAdd(nodeLeaf, slot, key, value);
    Node*   splitedNode = (NodeIsFull(nodeLeaf) ? LeafNodeSplit(nodeLeaf) : NULL);
    // Iterate back over nodes checking overflow / splitting
    while (!empty(qTree->stackNodes, qTree->stackNodesIndex)) {
        //        cout << slot << endl;
        InternalNode* node = pop(qTree->stackNodes, qTree->stackNodesIndex);
        if(node->node.maxValue == NULL || QueryRangeMaxGE(key, node->node.maxValue) ){
            node->node.maxValue = key;
        }
        if(node->node.minValue == NULL || QueryRangeMinGT(node->node.minValue, key) ){
            node->node.minValue = key;
        }
        slot = pop(qTree->stackSlots, qTree->stackSlotsIndex);
        //            System.out.println(key + ", "  + otherBound + "," + node.id + ", "  + node.keys[0] + "," + slot);
        if (splitedNode != NULL) {
            // split occurred in previous phase, splitedNode is new child
            KeyType * childKey = NodeSplitShiftKeysLeft(splitedNode);
            InternalNodeAdd(node, slot, childKey, splitedNode);

        }

        if(NodeIsFull(node)){
            splitedNode = NodeSplit(node);

        }else{
            splitedNode = NULL;
        }
        //    splitedNode = (node->isFull() ? node->split() : NULL);
    }



    qTree->elements++;
    if (splitedNode != NULL) {   // root was split, make new root
        QTreeMakeNewRoot(qTree, splitedNode);
    }
    return splitedNode;

}



void QTreeFindAndRemoveRelatedQueries(QTree* this, int attribute, Arraylist* removedQuery){
    Node* node = this->root;
    int slot = 0;
    KeyType  queryRange;
    KeyType* key = &queryRange;
    QueryRangeConstructorWithPara(key, attribute, attribute, true, true);
    QueryRange *removedMax, *removedMin;
    bool resetMax = false;
    bool resetMin = false;
    while (true) {
        while (!node->isLeaf){
            bool getNode = false;
            InternalNode* nodeInternal = (InternalNode*) node;
            for(slot = 0; slot <= nodeInternal->node.allocated; slot ++){
                if(QueryRangeMaxGE(nodeInternal->childs[slot]->maxValue, key)){
                    node = nodeInternal->childs[slot];
                    push(this->stackNodes, this->stackNodesIndex, nodeInternal);
                    push(this->stackSlots, this->stackSlotsIndex, slot);
                    getNode = true;
                    break;
                }
                if(slot < nodeInternal->node.allocated && QueryRangeMinGT( nodeInternal->childs[slot + 1]->minValue, key)){
                    break;
                }
            }
            //                System.out.println("getNode:" + getNode + ", node:" + node);
            if(!getNode){
                bool getAnother = false;
                while (!empty(this->stackNodes, this->stackNodesIndex)){
                    node = pop(this->stackNodes, this->stackNodesIndex);
                    slot = pop(this->stackSlots, this->stackSlotsIndex);
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
                        push(this->stackNodes, this->stackNodesIndex, internalNode);
                        push(this->stackSlots, this->stackSlotsIndex, slot + 1);
                        //                            System.out.println("another node:" + node);
                        slot = 0;
                        getAnother = true;
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
            resetMax = false;
            resetMin = false;
            for(int i = 0; i < leafNode->node.allocated ; i ++){
                //                    System.out.println("query:" + leafNode.values[i]);
                if(QueryMetaCover(leafNode->values[i], attribute)){
                    if(leafNode->node.maxValue == leafNode->node.keys[i]){
                        resetMax = true;
                        removedMax = (leafNode->node.maxValue);
                    }
                    if(leafNode->node.minValue == leafNode->node.keys[i]){
                        resetMin = true;
                        removedMin = leafNode->node.minValue;
                    }
                    ArraylistAdd(removedQuery, leafNode->values[i]);
                    this->elements --;
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
            if(empty(this->stackNodes, this->stackNodesIndex)){
                break;
            }
            bool getAnother = false;
            while (!empty(this->stackNodes, this->stackNodesIndex)){
                node = pop(this->stackNodes, this->stackNodesIndex);
                slot =pop(this->stackSlots, this->stackSlotsIndex);

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
                    push(this->stackNodes, this->stackNodesIndex, internalNode);
                    push(this->stackSlots, this->stackSlotsIndex, slot + 1);
                    slot = 0;
                    getAnother = true;
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
}


