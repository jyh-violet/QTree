//
// Created by workshop on 8/24/2021.
//

#include <query/QueryRange.h>
#include "QTree.h"

void NodeConstructor(Node* node, QTree *tree){
    node-> id = 0;
    node-> allocated = 0;
    node->tree = tree;
    node->keys = malloc(sizeof(KeyType *) * tree->Border);
    memset(node->keys,0, sizeof(KeyType *) * tree->Border);
}
void NodeDestroy(Node* node){
    free(node->keys);
    if(node->isLeaf){
        LeafNodeDestroy((LeafNode*)node);
    } else{
        InternalNodeDestroy((InternalNode*)node);
    }
}

extern SearchKeyType searchKeyType;

bool NodeIsUnderFlow(Node* node) {
    return (node->allocated <= (node->tree->Border >> 1));
}

bool NodeCanMerge( Node* this, Node* other) {
    return ((this->allocated + other->allocated) < this->tree->Border);
}

bool NodeIsFull(Node* node){ // node is full
    return (node->allocated >= node->tree->Border);
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

void setSearchKey(Node* node, KeyType * key){
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
                key->searchKey = (low + high) / 2;
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

