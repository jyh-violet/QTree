//
// Created by workshop on 9/5/2021.
//
#include "QTree.h"

void LeafNodeConstructor(LeafNode* leafNode, QTree *tree){
    NodeConstructor((Node*)leafNode, tree);
    leafNode->node.isLeaf = true;
    leafNode->values = malloc(sizeof (ValueType *) * tree->Border);
    memset(leafNode->values,0, sizeof (ValueType *) * tree->Border);
}
void LeafNodeDestroy(LeafNode* leafNode){
    free(leafNode->values);
}

void LeafNodeMerge(LeafNode* leafNode, InternalNode* nodeParent, int slot,
                   Node* nodeFROMx){
    LeafNode* nodeFROM = (LeafNode*) nodeFROMx;
    LeafNode* nodeTO = leafNode;
    int sizeTO = nodeTO->node.allocated;
    int sizeFROM = nodeFROM->node.allocated;
    // copy keys from nodeFROM to nodeTO
    memcpy(nodeTO->node.keys + sizeTO , nodeFROM->node.keys, sizeFROM * sizeof(KeyType *));
    memcpy(nodeTO->values + sizeTO , nodeFROM->values, (sizeFROM) * sizeof(ValueType *));

    nodeTO->node.allocated += sizeFROM; // keys of FROM and key of nodeParent
    if(!QueryRangeMaxGE(nodeTO->node.maxValue,nodeFROM->node.maxValue)){
        nodeTO->node.maxValue = nodeFROM->node.maxValue;
    }
    if(QueryRangeMinGT(nodeTO->node.minValue, nodeFROM->node.minValue)){
        nodeTO->node.minValue = nodeFROM->node.minValue;
    }

    // remove key from nodeParent
    InternalNodeRemove(nodeParent, slot);
    // Free nodeFROM
    NodeDestroy((Node*)nodeFROM);
    free(nodeFROM);

}

void * LeafNodRemove(LeafNode* leafNode, int slot) {
    return NULL;
}

bool LeafNodeAdd(LeafNode* leafNode, int slot, KeyType * newKey, ValueType * newValue){
    if (slot < leafNode->node.allocated) {
        memcpy(leafNode->node.keys + slot + 1, leafNode->node.keys + slot, (leafNode->node.allocated - slot) * sizeof(KeyType *));
        memcpy(leafNode->values + slot + 1, leafNode->values + slot, (leafNode->node.allocated - slot) * sizeof(ValueType *));
    }
    leafNode->node.allocated++;
    leafNode->node.keys[slot] = newKey;
    leafNode->values[slot] = newValue;
    if(leafNode->node.maxValue == NULL || (QueryRangeMaxGE(newKey, leafNode->node.maxValue))){
        leafNode->node.maxValue = newKey;
    }
    if(leafNode->node.minValue == NULL || (QueryRangeMinGT(leafNode->node.minValue, newKey))){
        leafNode->node.minValue = newKey;
    }
    return true;
}


void LeafNodeAllocId(LeafNode* leafNode) {
    leafNode->node.id = QTreeAllocNode(leafNode->node.tree, (true));
}

void LeafNodeResetMaxValue(LeafNode* leafNode) {
    if (leafNode->node.allocated == 0) {
        return;
    }
    leafNode->node.maxValue = leafNode->node.keys[0];
    for (int i = 1; i < leafNode->node.allocated; i++) {
        if (QueryRangeMaxGE((leafNode->node.keys[i]), leafNode->node.maxValue)) {
            leafNode->node.maxValue = leafNode->node.keys[i];
        }
    }
}


void LeafNodeResetMinValue(LeafNode* leafNode) {
    if (leafNode->node.allocated == 0) {
        return;
    }
    leafNode->node.minValue = leafNode->node.keys[0];
    for (int i = 1; i < leafNode->node.allocated; i++) {
        if (QueryRangeMinGT((leafNode->node.minValue), leafNode->node.keys[i])) {
            leafNode->node.minValue = leafNode->node.keys[i];
        }
    }
}

Node* LeafNodeSplit(LeafNode* leafNode) {
    LeafNode* newHigh = malloc(sizeof (LeafNode));
    LeafNodeConstructor(newHigh, leafNode->node.tree);
    LeafNodeAllocId(newHigh);

    int j = leafNode->node.allocated >> 1; // dividir por dos (libro)
    int newsize = leafNode->node.allocated - j;
    // if (log.isDebugEnabled()) log.debug("split j=" + j);
    memcpy(newHigh->node.keys, leafNode->node.keys + j,  newsize * sizeof (KeyType *));

    memcpy(newHigh->values, leafNode->values + j,  newsize * sizeof (ValueType *));

    for (int i = j; i < j + newsize; i++) {
        leafNode->node.keys[i] = NULL;
        // clear bound id of the influenced query
        leafNode->values[i] = NULL;
    }
    newHigh->node.allocated = newsize;
    leafNode->node.allocated -= newsize;
    if(QueryRangeGT(leafNode->node.maxValue, newHigh->node.keys[0])){
        newHigh->node.maxValue = leafNode->node.maxValue;
        LeafNodeResetMaxValue(leafNode);
    }else {
        LeafNodeResetMaxValue(newHigh);
    }
    if(QueryRangeGT(leafNode->node.minValue, newHigh->node.keys[0])){
        newHigh->node.minValue = leafNode->node.minValue;
        LeafNodeResetMinValue(leafNode);
    }else {
        LeafNodeResetMinValue(newHigh);
    }
    return (Node*)newHigh;
}

KeyType * LeafNodeSplitShiftKeysLeft(LeafNode* leafNode) {
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
    printf("[L%d](%d)(%d){", leafNode->node.id, leafNode->node.allocated, ((QueryRange*)(leafNode->node.maxValue))->upper);
    for (int i = 0; i < leafNode->node.allocated; i++) {
        QueryRange * k = (QueryRange *)leafNode->node.keys[i];
        QueryMeta* v = (QueryMeta *)leafNode->values[i];
        printQueryRange(k);
        printf("Q[%s] | ",v->queryId );
    }
    printf("}");
}