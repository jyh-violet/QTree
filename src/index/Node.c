//
// Created by workshop on 8/24/2021.
//

#include <query/QueryRange.h>
#include "QTree.h"

void NodeConstructor(Node* node, QTree *tree){
    node-> id = 0;
    node-> allocated = 0;
    node->tree = tree;
//    node->keys = malloc(sizeof(KeyType *) * tree->Border);
    memset(node->keys,0, sizeof(KeyType *) * Border);
}
void NodeDestroy(Node* node){
//    free(node->keys);
    if(node->isLeaf){
        LeafNodeDestroy((LeafNode*)node);
    } else{
        InternalNodeDestroy((InternalNode*)node);
    }
}

bool NodeIsUnderFlow(Node* node) {
    return (node->allocated <= (Border >> 1));
}

bool NodeCanMerge( Node* this, Node* other) {
    return ((this->allocated + other->allocated) < Border);
}

bool NodeIsFull(Node* node){ // node is full
    return (node->allocated >= Border);
}

int NodeFindSlotByKey( Node* node, KeyType* searchKey) {
    // return Arrays.binarySearch(keys, 0, allocated, searchKey);
    if(node->allocated == 0){
        return -1;
    }
    int low = 0;
    int high = node->allocated - 1;

    while (low <= high) {
        int mid = (low + high) >> 1;
        KeyType *midVal = (node->keys[mid]);

        if (QueryRangeLT(midVal, searchKey)) {
            low = mid + 1;
        } else if (QueryRangeGT(midVal, searchKey)) {
            high = mid - 1;
        } else {
            return mid; // key found
        }
    }
    return -(low + 1);  // key not found.
}

void printNode(Node* node){
    if(node->isLeaf){
        printLeafNode((LeafNode*)node);
    }else{
        printInternalNode((InternalNode*)node);
    }
}

Node* NodeSplit(Node* node){
    if(node->isLeaf){
       return LeafNodeSplit(node);
    } else{
        return InternalNodeSplit(node);
    }
}

int NodeGetHeight(Node* node) {
    if(node->isLeaf){
        return LeafNodeGetHeight(node);
    } else{
        return InternalNodeGetHeight(node);
    }
}

void NodeResetId(Node* node) {
    if(node->isLeaf){
        LeafNodeResetId(node);
    } else{
        InternalNodeResetId(node);
    }
}

void NodeMerge(Node* this, InternalNode* nodeParent, int slot,
                    Node* nodeFROM){
    if(this->isLeaf){
        LeafNodeMerge(this, nodeParent, slot, nodeFROM);
    } else{
        InternalNodeMerge(this, nodeParent, slot, nodeFROM);
    }
}

void * NodeSplitShiftKeysLeft(Node* this){
    if(this->isLeaf){
        LeafNodeSplitShiftKeysLeft(this);
    } else{
        InternalNodeSplitShiftKeysLeft(this);
    }
}

