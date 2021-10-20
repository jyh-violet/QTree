//
// Created by workshop on 8/24/2021.
//

#include <query/QueryRange.h>
#include <pthread.h>
#include "QTree.h"

void NodeConstructor(Node* node, QTree *tree){
    node-> id = 0;
    node-> allocated = 0;
    node->tree = tree;
    pthread_spin_init(&node->lock, PTHREAD_PROCESS_SHARED);
    node->nextNodeMin = RAND_MAX;
    node->read = 0;

//    node->keys = malloc(sizeof(KeyType *) * tree->Border);
//    memset(node->keys,0, sizeof(KeyType *) * Border);
}
void NodeDestroy(Node* node){
//    free(node->keys);
    pthread_spin_destroy(&node->lock);
    if(NodeIsLeaf(node)){
        LeafNodeDestroy((LeafNode*)node);
    } else{
        InternalNodeDestroy((InternalNode*)node);
    }
}

void NodeCheckTree(Node* node){
    if(node->tree == NULL){
        printNode(node);
    }
    if(NodeIsLeaf(node)){
        for(int i = 0; i <= node->allocated; i ++){
            NodeCheckTree(((InternalNode*)node)->childs[i]);
        }
    }
}

BOOL NodeIsUnderFlow(Node* node) {
    return (BOOL)(node->allocated <= (Border >> 1));
}

BOOL NodeCanMerge( Node* node, Node* other) {
    return (BOOL)((node->allocated + other->allocated + 1) < Border);
}

BOOL NodeIsFull(Node* node){ // node is full
    return (BOOL)(node->allocated >= Border);
}



void printNode(Node* node){
    if(NodeIsLeaf(node)){
        printLeafNode((LeafNode*)node);
    }else{
        printInternalNode((InternalNode*)node);
    }
}

Node* NodeSplit(Node* node){
    if(NodeIsLeaf(node)){
       return LeafNodeSplit((LeafNode*)node);
    } else{
        return InternalNodeSplit((InternalNode *)node);
    }
}

int NodeGetHeight(Node* node) {
    if(NodeIsLeaf(node)){
        return LeafNodeGetHeight((LeafNode*)node);
    } else{
        return InternalNodeGetHeight((InternalNode *)node);
    }
}

void NodeResetId(Node* node) {
    if(NodeIsLeaf(node)){
        LeafNodeResetId((LeafNode*)node);
    } else{
        InternalNodeResetId((InternalNode *)node);
    }
}

void NodeMerge(Node* node, InternalNode* nodeParent, int slot,
                    Node* nodeFROM){
    if(NodeIsLeaf(node)){
        LeafNodeMerge((LeafNode*)node, nodeParent, slot, nodeFROM);
    } else{
        InternalNodeMerge(node, nodeParent, slot, nodeFROM);
    }
}

KeyType  NodeSplitShiftKeysLeft(Node* node){
    if(NodeIsLeaf(node)){
        return LeafNodeSplitShiftKeysLeft((LeafNode*)node);
    } else{
        return InternalNodeSplitShiftKeysLeft((InternalNode *)node);
    }
}

BOOL NodeCheckMaxMin(Node * node){
    if(NodeIsLeaf(node)){
        return LeafNodeCheckMaxMin((LeafNode*)node);
    } else{
        return InternalNodeCheckMaxMin((InternalNode *)node);
    }
}

BOOL NodeCheckKey(Node * node){
    if(NodeIsLeaf(node)){
        return LeafNodeCheckKey((LeafNode*)node);
    } else{
        return InternalNodeCheckKey((InternalNode *)node);
    }
}

BOOL NodeCheckLink(Node* node){
    if(NodeIsLeaf(node)){
        return LeafNodeCheckLink((LeafNode*)node);
    } else{
        return InternalNodeCheckLink((InternalNode *)node);
    }
}
void NodeAddWriteLock(Node* node){
    pthread_spin_lock(&((Node*)node)->lock);
    vmlog("addWriteLock node:%d", ((Node*)node)->id);
    while(((Node*)node)->read > 0){}
}
void NodeRmWriteLock(Node* node){
    pthread_spin_unlock(&((Node*)node)->lock);
    vmlog("rmWriteLock node:%d", ((Node*)node)->id);
}
