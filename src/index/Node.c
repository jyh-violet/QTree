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
    node->insertRead = 0;

//    node->keys = malloc(sizeof(KeyType *) * tree->Border);
//    memset(node->keys,0, sizeof(KeyType *) * Border);
}
void NodeDestroy(Node* node){
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
    if(node->allocated > Border + 1){
        return TRUE;
    }
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

    vmlog(InsertLog,"addWriteLock node:%d", ((Node*)node)->id);

    pthread_spin_lock(&((Node*)node)->lock);
    while (node->insertRead > 0){}
    vmlog(InsertLog,"addWriteLock node:%d success", ((Node*)node)->id);
}
void NodeRmWriteLock(Node* node){
    vmlog(InsertLog,"rmWriteLock node:%d", ((Node*)node)->id);
    pthread_spin_unlock(&((Node*)node)->lock);
    vmlog(InsertLog,"rmWriteLock node:%d suceess", ((Node*)node)->id);
}

void NodeAddRWLock(Node* node){
    vmlog(InsertLog,"addRWLock node:%d", ((Node*)node)->id);
//    if(!NodeIsLeaf(node)){
//        pthread_rwlock_rdlock(&((InternalNode*)node)->removeLock);
//    }
//    node->read ++;
    pthread_spin_lock(&((Node*)node)->lock);
    while (node->insertRead > 0){}
    vmlog(InsertLog,"addRWLock node:%d success", ((Node*)node)->id);
}
void NodeRmRWLock(Node* node){
    vmlog(InsertLog,"rmRWLock node:%d", ((Node*)node)->id);
    pthread_spin_unlock(&((Node*)node)->lock);
//    if(!NodeIsLeaf(node)){
//        pthread_rwlock_unlock(&((InternalNode*)node)->removeLock);
//    }
//    node->read --;
    vmlog(InsertLog,"rmRWLock node:%d sucees", ((Node*)node)->id);

}
void NodeRmReadLock(Node* node){
//    vmlog(InsertLog,"rmReadLock node:%d", ((Node*)node)->id);
//    if(!NodeIsLeaf(node)){
//        pthread_rwlock_unlock(&((InternalNode*)node)->removeLock);
//    }
//    node->read --;
//    vmlog(InsertLog,"rmReadLock node:%d success", ((Node*)node)->id);
}

void NodeAddReadLock(Node* node){
//    vmlog(InsertLog,"addReadLock node:%d", ((Node*)node)->id);
//    if(!NodeIsLeaf(node)){
//        pthread_rwlock_rdlock(&((InternalNode*)node)->removeLock);
//    }
//    node->read ++;
//    vmlog(InsertLog,"addReadLock node:%d succes", ((Node*)node)->id);

}
void NodeAddInsertReadLock(Node* node){
    vmlog(InsertLog,"NodeAddInsertReadLock node:%d", ((Node*)node)->id);
    int read = __sync_fetch_and_add(&node->insertRead, 1);
    if(read == 0){
        while (!pthread_spin_trylock(&((Node*)node)->lock)){}
    }

    vmlog(InsertLog,"NodeAddInsertReadLock node:%d success, read:%d", ((Node*)node)->id, read);
}
void NodeRmInsertReadLock(Node* node){
    vmlog(InsertLog,"NodeRmInsertReadLock node:%d", ((Node*)node)->id);
    int read = __sync_fetch_and_sub(&node->insertRead, 1);
    if(read == 1){
       pthread_spin_unlock(&((Node*)node)->lock);
    }
    vmlog(InsertLog,"NodeRmInsertReadLock node:%d success, read:%d", ((Node*)node)->id, read);
}

void NodeAddInsertRWLock(Node* node){
    vmlog(InsertLog,"NodeAddInsertRWLock node:%d", ((Node*)node)->id);
    pthread_spin_lock(&((Node*)node)->lock);
    __sync_fetch_and_add(&node->insertRead, 1);
    vmlog(InsertLog,"NodeAddInsertRWLock node:%d success", ((Node*)node)->id);
}

void NodeRmInsertRWLock(Node* node){
    vmlog(InsertLog,"NodeRmInsertRWLock node:%d", ((Node*)node)->id);
    __sync_fetch_and_sub(&node->insertRead, 1);
    pthread_spin_unlock(&((Node*)node)->lock);
    vmlog(InsertLog,"NodeRmInsertRWLock node:%d success", ((Node*)node)->id);
}
