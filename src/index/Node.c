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
    node->nextNodeMin = RAND_MAX;
    node->insertLock = 0;
    node->removeLock = 0;

//    node->keys = malloc(sizeof(KeyType *) * tree->Border);
//    memset(node->keys,0, sizeof(KeyType *) * Border);
}
void NodeDestroy(Node* node){
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
        printNode(node);
        exit(-1);
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
inline void NodeAddInsertWriteLock(Node* node){

    vmlog(InsertLog,"addWriteLock node:%d", ((Node*)node)->id);

    int try = 0;
    while (1){
        if(__sync_bool_compare_and_swap(&node->insertLock, 0, 1)){
            vmlog(InsertLog,"addWriteLock node:%d success", ((Node*)node)->id);
            return;
        } else{
            try ++;
            if(try%1000000000 == 0){
                vmlog(WARN,"addWriteLock node:%d conflict:%x", ((Node*)node)->id, node->insertLock);
//                exit(-1);
            }
        }
    }

}
#define TryCount 10
BOOL NodeTryAddInsertWriteLock(Node* node){
//    vmlog(InsertLog,"NodeTryAddWriteLock node:%d", ((Node*)node)->id);
    int try = 0;
    while (try < TryCount){
        if(__sync_bool_compare_and_swap(&node->insertLock, 0, 1)){
//            vmlog(InsertLog,"NodeTryAddWriteLock node:%d success", ((Node*)node)->id);
            return TRUE;
        } else{
            try ++;
        }
    }

//    vmlog(MiXLog,"NodeTryAddWriteLock node:%d failed:%x", ((Node*)node)->id, node->insertLock);
    return FALSE;
}

void NodeRmInsertWriteLock(Node* node){
//    vmlog(InsertLog,"rmWriteLock node:%d", ((Node*)node)->id);
    __sync_bool_compare_and_swap(&node->insertLock, 1, 0);
    vmlog(InsertLog,"rmWriteLock node:%d suceess", ((Node*)node)->id);
}


void NodeDegradeInsertLock(Node* node, int threadId){
    vmlog(InsertLog,"rmWriteLock node:%d", ((Node*)node)->id);
    u_int64_t target =  (1 << (threadId + 1));
    __sync_bool_compare_and_swap(&node->insertLock, 1, target);
    vmlog(InsertLog,"rmWriteLock node:%d suceess", ((Node*)node)->id);
}


void NodeAddInsertReadLock(Node* node, int threadId){
    vmlog(InsertLog,"NodeAddInsertReadLock node:%d", ((Node*)node)->id);
    //    int tid = getThreadId();
    while (1){
        u_int64_t  flag = node->insertLock;
        if(flag != 1){
            u_int64_t target = flag | (1 << (threadId + 1));
            if(__sync_bool_compare_and_swap(&node->insertLock, flag, target)){
                vmlog(InsertLog,"NodeAddInsertReadLock node:%d success", ((Node*)node)->id);
                return;
            }
        }
    }
}

BOOL NodeTryAddInsertReadLock(Node* node, int threadId){
    //    vmlog(InsertLog,"NodeTryAddInsertReadLock node:%d", ((Node*)node)->id);
    //    int tid = getThreadId();
    int try = 0;
    while (try < TryCount){
        u_int64_t  flag = node->insertLock;
        if(flag != 1){
            u_int64_t target = flag | (1 << (threadId + 1));
            if(__sync_bool_compare_and_swap(&node->insertLock, flag, target)){
                //                vmlog(InsertLog,"NodeTryAddInsertReadLock node:%d success", ((Node*)node)->id);
                return TRUE;
            }
        } else{
            try ++;
        }
    }
    //    vmlog(MiXLog,"NodeTryAddInsertReadLock node:%d failed:%x", ((Node*)node)->id, node->insertLock);
    return FALSE;
}
void NodeRmInsertReadLock(Node* node, int threadId){
    //    int tid = getThreadId();
    //    vmlog(InsertLog,"NodeRmInsertReadLock node:%d", ((Node*)node)->id);
    __sync_and_and_fetch(&node->insertLock, ~(1<<(threadId + 1)));
    vmlog(InsertLog,"NodeRmInsertReadLock node:%d success", ((Node*)node)->id);
}

void NodeRmInsertReadLockNoLog(Node* node, int threadId){
    //    int tid = getThreadId();
    //    vmlog(InsertLog,"NodeRmInsertReadLock node:%d", ((Node*)node)->id);
    __sync_and_and_fetch(&node->insertLock, ~(1<<(threadId + 1)));
    //    vmlog(InsertLog,"NodeRmInsertReadLock node:%d success", ((Node*)node)->id);
}

#define RemoveReadOff 2

void NodeAddRemoveReadInsertWriteLock(Node* node, int threadId){
    NodeAddRemoveReadLock(node, threadId);
    NodeAddInsertWriteLock(node);

}
void NodeRmRemoveReadInsertWriteLock(Node* node, int threadId){
    vmlog(InsertLog,"NodeRmRemoveReadInsertWriteLock node:%d", ((Node*)node)->id);
    __sync_bool_compare_and_swap(&node->insertLock, 1, 0);
    __sync_and_and_fetch(&node->removeLock, ~(1<<(threadId + RemoveReadOff)));
    vmlog(RemoveLog,"NodeRmRemoveReadInsertWriteLock node:%d success :%x", ((Node*)node)->id, node->removeLock);

}


void NodeRmRemoveReadLock(Node* node, int threadId){
    __sync_and_and_fetch(&node->removeLock, ~(1<<(threadId + RemoveReadOff)));
    vmlog(RemoveLog,"NodeRmRemoveReadLock node:%d success :%x", ((Node*)node)->id, node->removeLock);
}

inline void NodeAddRemoveReadLock(Node* node, int threadId){
    vmlog(RemoveLog,"NodeAddRemoveReadLock node:%d ", ((Node*)node)->id);
    //    int tid = getThreadId();
    int try = 0;
    while (1){
        u_int64_t  flag = node->removeLock;
        if((flag & 1) == 0){
            u_int64_t target = flag | (1 << (threadId + RemoveReadOff));
            if(__sync_bool_compare_and_swap(&node->removeLock, flag, target)){
                vmlog(RemoveLog,"NodeAddRemoveReadLock node:%d success :%x", ((Node*)node)->id, node->removeLock);
                return;
            }
        } else{
            try ++;
            if(try%1000000000 == 0){
                vmlog(WARN,"NodeAddRemoveReadLock node:%d conflict:%x", ((Node*)node)->id, node->removeLock);
            }
        }
    }
}

void NodeAddRemoveWriteLock(Node* node){
    vmlog(RemoveLog,"NodeAddRemoveWriteLock node:%d ", ((Node*)node)->id);
    __sync_fetch_and_or (&node->removeLock, 1);
    int try = 0;
    while (1){
        if(__sync_bool_compare_and_swap(&node->removeLock, 1, 3)){
            vmlog(RemoveLog,"NodeAddRemoveWriteLock node:%d success", ((Node*)node)->id);
            return;
        } else{
            __sync_fetch_and_or (&node->removeLock, 1);
            try ++;
            if(try%1000000000 == 0){
                vmlog(WARN,"NodeAddRemoveWriteLock node:%d conflict:%x", ((Node*)node)->id, node->removeLock);
                                exit(-1);
            }
        }
    }
}

void NodeRmRemoveWriteLock(Node* node){
    __sync_bool_compare_and_swap(&node->removeLock, 3, 0);
    vmlog(RemoveLog,"NodeRmRemoveWriteLock node:%d suceess", ((Node*)node)->id);
}

BOOL NodeTryAddRemoveWriteLock(Node* node){
    vmlog(RemoveLog,"NodeTryAddRemoveWriteLock node:%d", ((Node*)node)->id);
    int try = 0;
    if((node->removeLock & 1) == 1){
        vmlog(RemoveLog,"NodeTryAddRemoveWriteLock node:%d got by another thread :%x", ((Node*)node)->id, node->removeLock);
        return FALSE;
    } else{
        __sync_fetch_and_or (&node->removeLock, 1);
    }
    while (1){
        if(__sync_bool_compare_and_swap(&node->removeLock, 1, 3)){
            vmlog(RemoveLog,"NodeTryAddRemoveWriteLock node:%d success", ((Node*)node)->id);
            return TRUE;
        }else{
            try ++;
            __sync_fetch_and_or (&node->removeLock, 1);
            if(try%1000000000 == 0){
                vmlog(WARN,"NodeTryAddRemoveWriteLock node:%d conflict:%x", ((Node*)node)->id, node->removeLock);
                exit(-1);
            }
        }
    }
    vmlog(RemoveLog,"NodeTryAddRemoveWriteLock node:%d failed:%x", ((Node*)node)->id, node->removeLock);
    return FALSE;
}

int getThreadId(){
    return 1;
//    int tid = (int )pthread_getspecific(threadId);
//    vmlog(MiXLog, "getThreadId:%d", tid);
//    return tid;
}

BoundKey NodeGetMaxValue(Node* node){
    BoundKey key = __sync_fetch_and_add (&node->maxValue, 0);
    return key;
}

BoundKey NodeGetMinValue(Node* node){
    BoundKey key = __sync_fetch_and_add (&node->minValue, 0);
    return key;
}