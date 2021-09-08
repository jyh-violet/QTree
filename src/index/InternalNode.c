//
// Created by workshop on 9/5/2021.
//
#include <string.h>
#include <query/QueryRange.h>
#include "QTree.h"

void InternalNodeConstructor(InternalNode* internalNode, QTree* qTree){
    memset(internalNode,0, sizeof(InternalNode));
    NodeConstructor((Node*)internalNode, qTree);
    internalNode->node.isLeaf = FALSE;
//    internalNode->childs = malloc(sizeof (Node*) * qTree->Border + 1);

}
void InternalNodeDestroy(InternalNode* internalNode){
    for(int i = 0; i <= internalNode->node.allocated; i++){
        NodeDestroy(internalNode->childs[i]);
    }
    free(internalNode);
//    free(internalNode->childs);
}


BOOL InternalNodeAdd(InternalNode* internalNode, int slot, KeyType * newKey, Node* child){
    if (slot < internalNode->node.allocated) {
        memcpy(internalNode->node.keys + slot + 1, internalNode->node.keys + slot, (internalNode->node.allocated - slot) * sizeof(KeyType));
        memcpy(internalNode->childs + slot + 2, internalNode->childs + slot + 1, (internalNode->node.allocated - slot) * sizeof(Node*));
    }
    internalNode->node.allocated++;
    internalNode->node.keys[slot] = *newKey;
    internalNode->childs[slot + 1] = child;
    return TRUE;
}


void InternalNodeResetMaxValue(InternalNode* internalNode){
    internalNode->node.maxValue = internalNode->childs[0]->maxValue;
    for(int i = 1; i <= internalNode->node.allocated; i ++){
        if( QueryRangeMaxGE((internalNode->childs[i]->maxValue), internalNode->node.maxValue)){
            internalNode->node.maxValue = internalNode->childs[i]->maxValue;
        }
    }
}

void InternalNodeResetMinValue(InternalNode* internalNode){
    internalNode->node.minValue = internalNode->childs[0]->minValue;
    for(int i = 1; i <= internalNode->node.allocated; i ++){
        if(QueryRangeMinGT ((internalNode->node.minValue), internalNode->childs[i]->minValue)){
            internalNode->node.minValue = internalNode->childs[i]->minValue;
        }
    }
}

void InternalNodeAllocId(InternalNode* internalNode) {
    internalNode->node.id = QTreeAllocNode(internalNode->node.tree, (FALSE));
}
Node* InternalNodeSplit(InternalNode* internalNode) {
    if(internalNode->node.tree == NULL){
        printInternalNode(internalNode);
    }
    internalNode->node.tree->internalSplitCount ++;
    InternalNode* newHigh = (InternalNode* )malloc(sizeof (InternalNode));
    InternalNodeConstructor(newHigh, internalNode->node.tree);
    InternalNodeAllocId(newHigh);
    // int j = ((allocated >> 1) | (allocated & 1)); // dividir por dos y sumar el resto (0 o 1)
    int j = (internalNode->node.allocated >> 1); // dividir por dos (libro)
    int newsize = internalNode->node.allocated - j;

    // if (log.isDebugEnabled()) log.debug("split j=" + j);
    memcpy(newHigh->node.keys, internalNode->node.keys + j,  newsize * sizeof (KeyType ));

    memcpy(newHigh->childs, internalNode->childs + (j+1),  newsize * sizeof (Node*));

    memset(internalNode->node.keys + j, 0, newsize * sizeof (KeyType ));
    memset(internalNode->childs + j + 1, 0, newsize * sizeof (Node *));
//    for (int i = j; i < j + newsize; i++) {
//        internalNode->node.keys[i] = NULL;
//        internalNode->childs[i + 1] = NULL;
//    }
    newHigh->node.allocated = newsize;
    internalNode->node.allocated -= newsize;

    InternalNodeResetMaxValue(internalNode);
    InternalNodeResetMinValue(internalNode);
    newHigh->node.allocated --;
    InternalNodeResetMaxValue(newHigh);
    InternalNodeResetMinValue(newHigh);
    newHigh->node.allocated ++;
    return (Node*)newHigh;
}


KeyType InternalNodeSplitShiftKeysLeft(InternalNode* internalNode) {
    KeyType removed = internalNode->node.keys[0];
    memcpy(internalNode->node.keys, internalNode->node.keys + 1, (internalNode->node.allocated - 1) * sizeof(KeyType ));
    internalNode->node.allocated--;
//    internalNode->node.keys[internalNode->node.allocated] = NULL;
    internalNode->childs[internalNode->node.allocated + 1] = NULL;
    return removed;
}

int InternalNodeGetId(InternalNode* internalNode) {
    return internalNode->node.id;
}

int InternalNodeGetHeight(InternalNode* internalNode) {
    int h = 0;
    for (int i = 0; i <= internalNode->node.allocated ; ++i) {
        NodeGetHeight(internalNode->childs[i]);
    }
    return 0;
}

void InternalNodeResetId(InternalNode* internalNode){
    for (int i = 0; i <= internalNode->node.allocated ; ++i) {
        NodeResetId(internalNode->childs[i]);
    }
}

BOOL InternalNodeCheckUnderflowWithRight(InternalNode* internalNode, int slot){
    Node* nodeLeft = internalNode->childs[slot];
    while ((slot < internalNode->node.allocated) &&NodeIsUnderFlow(nodeLeft)) {
        Node* nodeRight = internalNode->childs[slot + 1];
        if (NodeCanMerge(nodeLeft, nodeRight)) {
            NodeMerge(nodeLeft, internalNode, slot, nodeRight);
            internalNode->childs[slot] = nodeLeft;
        } else {
            //                nodeLeft.shiftRL(internalNode, slot, nodeRight);
        }

        return TRUE;
    }
    return FALSE;
}

KeyType InternalNodeRemove(InternalNode* internalNode, int slot) {
    if (slot < -1) {
        printf("faking slot=%d allocated=%d\n", slot, internalNode->node.allocated);
        exit(-2);
    }
    KeyType * removedUpper = internalNode->childs[slot + 1]->maxValue;

    if(slot == -1){
        memcpy(internalNode->node.keys, internalNode->node.keys + 1, (internalNode->node.allocated - 1) * sizeof (KeyType ));
        memcpy(internalNode->childs, internalNode->childs + 1, (internalNode->node.allocated) * sizeof (Node*));
    }else if (slot < internalNode->node.allocated) {
        memcpy(internalNode->node.keys + slot, internalNode->node.keys + slot + 1, (internalNode->node.allocated - slot - 1) * sizeof (KeyType ));
        memcpy(internalNode->childs + slot + 1, internalNode->childs + slot + 2, (internalNode->node.allocated - slot - 1) * sizeof (Node*));
    }
    if (internalNode->node.allocated > 0) {
        internalNode->node.allocated--;
    }
//    internalNode->node.keys[internalNode->node.allocated] = NULL;
    internalNode->childs[internalNode->node.allocated + 1] = NULL;
    return *removedUpper;
}

void InternalNodeMerge(Node* internalNode, InternalNode* nodeParent, int slot, Node* nodeFROMx) {
    InternalNode* nodeFROM = (InternalNode*) nodeFROMx;
    InternalNode* nodeTO = (InternalNode*)internalNode;
    int sizeTO = nodeTO->node.allocated;
    int sizeFROM = nodeFROM->node.allocated;
    // copy keys from nodeFROM to nodeTO
    memcpy(nodeTO->node.keys + sizeTO + 1, nodeFROM->node.keys, sizeFROM * sizeof(KeyType));
    memcpy(nodeTO->childs + sizeTO + 1, nodeFROM->childs, (sizeFROM + 1) * sizeof(Node*));
    // add key to nodeTO
    nodeTO->node.keys[sizeTO] = nodeParent->node.keys[slot];
    nodeTO->node.allocated += sizeFROM + 1; // keys of FROM and key of nodeParent
    if(!QueryRangeMaxGE(nodeTO->node.maxValue, nodeFROM->node.maxValue)){
        nodeTO->node.maxValue = nodeFROM->node.maxValue;
    }
    if(QueryRangeMinGT(nodeTO->node.minValue, nodeFROM->node.minValue)){
        nodeTO->node.minValue = nodeFROM->node.minValue;
    }

    // remove key from nodeParent
    InternalNodeRemove(nodeParent, slot);
    // Free nodeFROM
    free((Node*)nodeFROM);
}



void printInternalNode(InternalNode* internalNode){
    printf("[I%d](%d)(%d,%d){", internalNode->node.id, internalNode->node.allocated,
           ((QueryRange*)(internalNode->node.minValue))->lower,  ((QueryRange*)(internalNode->node.maxValue))->upper);
    for (int i = 0; i < internalNode->node.allocated; i++) {
        QueryRange * k = &internalNode->node.keys[i];
        if (i == 0) { // left
            printf("C%d:Node%d<", i, internalNode->childs[i]->id);
        } else {
            printf("<");
        }
        printf("%d", k->searchKey);
        printf(">C%d:Node%d<",  i + 1, internalNode->childs[i + 1]->id);

    }
    printf("}\n");
}
