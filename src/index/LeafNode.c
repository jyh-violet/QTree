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
    // copy keys from nodeFROM to nodeTO
    memcpy(nodeTO->node.keys + sizeTO , nodeFROM->node.keys, sizeFROM * sizeof(KeyType ));
    memcpy(nodeTO->values + sizeTO , nodeFROM->values, (sizeFROM) * sizeof(ValueType *));

    nodeTO->node.allocated += sizeFROM; // keys of FROM and key of nodeParent
    if((nodeTO->node.maxValue)< (nodeFROM->node.maxValue)){
        nodeTO->node.maxValue = nodeFROM->node.maxValue;
    }
    if((nodeTO->node.minValue) > (nodeFROM->node.minValue)){
        nodeTO->node.minValue = nodeFROM->node.minValue;
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

Node* LeafNodeSplit(LeafNode* leafNode) {
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

KeyType  LeafNodeSplitShiftKeysLeft(LeafNode* leafNode) {
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
