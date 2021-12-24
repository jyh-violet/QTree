//
// Created by workshop on 8/24/2021.
//

#include <query/QueryRange.h>
#include <pthread.h>
#include "QTree.h"

#define WriteLockTryThreshold 100000000L
#define TryCount 10

void NodeConstructor(Node* node, QTree *tree){
    node-> id = 0;
    node-> allocated = 0;
    node->tree = tree;
    node->nextNodeMin = RAND_MAX;
    node->removeLock = 0;
    pthread_rwlock_init(&node->insertLock, NULL);

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
    return (BOOL)(NodeGetAllocated(node) <= (Border >> 1));
}

BOOL NodeCanMerge( Node* node, Node* other) {
    return (BOOL)((NodeGetAllocated(node) + NodeGetAllocated(other) + 1) < Border);
}

BOOL NodeIsFull(Node* node){ // node is full
    if(node->allocated > Border + 1){
        printNode(node);
        vmlog(ERROR, "nodeIsFull error");
    }
    return (BOOL)(NodeGetAllocated(node) >= Border);
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

BOOL NodeMerge(Node* node, InternalNode* nodeParent, int slot,
                    Node* nodeFROM){
    Node* rightNode = nodeFROM->right;
    if((rightNode != NULL) && (NodeTryAddInsertWriteLock(rightNode) == FALSE)){
        return FALSE;
    }
    if(NodeIsLeaf(node)){
        LeafNodeMerge((LeafNode*)node, nodeParent, slot, nodeFROM);
    } else{
        InternalNodeMerge(node, nodeParent, slot, nodeFROM);
    }
    if(rightNode != NULL){
        NodeRmInsertWriteLock(rightNode);
    }
    return TRUE;
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

#define RemoveReadOff 2

inline void NodeAddInsertWriteLock(Node* node){
    vmlog(RemoveLog,"NodeAddInsertWriteLock node:%d", ((Node*)node)->id);
    pthread_rwlock_wrlock(&node->insertLock);
    vmlog(RemoveLog,"NodeAddInsertWriteLock node:%d success", ((Node*)node)->id);

}
BOOL NodeTryAddInsertWriteLock(Node* node){
    vmlog(RemoveLog,"NodeTryAddWriteLock node:%d", ((Node*)node)->id);
    if(pthread_rwlock_trywrlock(&node->insertLock) == 0){
        vmlog(RemoveLog,"NodeTryAddWriteLock node:%d success", ((Node*)node)->id);
        return TRUE;
    } else{
        vmlog(RemoveLog,"NodeTryAddWriteLock node:%d failed:%x", ((Node*)node)->id, node->insertLock);
        return FALSE;
    }

}

BOOL NodeTryAddInsertWriteLockForRemove(Node* node){
//    vmlog(RemoveLog,"NodeTryAddInsertWriteLockForRemove node:%d ", ((Node*)node)->id);
    int try = 0;
    while (TRUE){
        LockType  flag = node->removeFlag;
        if((flag & 1)  == 1){
            return FALSE;
        }
        if( __sync_bool_compare_and_swap (&node->removeFlag, 0, 1)){
            break;
        } else{
            try ++;
            if(try == TryCount){
//                vmlog(RemoveLog,"NodeTryAddInsertWriteLockForRemove node:%d failed:%x", ((Node*)node)->id, node->insertLock);
                return FALSE;
            }
        }
    }
    pthread_rwlock_wrlock(&node->insertLock);
    vmlog(RemoveLog,"NodeTryAddInsertWriteLockForRemove node:%d success", ((Node*)node)->id);
    return TRUE;
}

void NodeRmInsertWriteLockForRemove(Node* node){
//    vmlog(InsertLog,"NodeRmInsertWriteLockForRemove node:%d", ((Node*)node)->id);
    __sync_bool_compare_and_swap(&node->removeFlag, 1, 0);
    pthread_rwlock_unlock(&node->insertLock);
    vmlog(RemoveLog,"NodeRmInsertWriteLockForRemove node:%d suceess:%x", ((Node*)node)->id, node->insertLock);

}

void NodeRmInsertWriteLock(Node* node){
//    vmlog(InsertLog,"rmWriteLock node:%d", ((Node*)node)->id);
    pthread_rwlock_unlock(&node->insertLock);
    vmlog(RemoveLog,"rmWriteLock node:%d suceess :%x", ((Node*)node)->id, node->insertLock);
}


void NodeDegradeInsertLock(Node* node, int threadId){

}


void NodeAddInsertReadLock(Node* node, int threadId){
    vmlog(RemoveLog,"NodeAddInsertReadLock node:%d", ((Node*)node)->id);
    //    int tid = getThreadId();
    pthread_rwlock_rdlock(&node->insertLock);
    vmlog(RemoveLog,"NodeAddInsertReadLock node:%d success" , ((Node*)node)->id);
}

BOOL NodeTryAddInsertReadLock(Node* node, int threadId){
    vmlog(RemoveLog,"NodeTryAddInsertReadLock node:%d", ((Node*)node)->id);
    //    int tid = getThreadId();
    int res = pthread_rwlock_tryrdlock(&node->insertLock);
    if(res == 0){
        vmlog(RemoveLog,"NodeTryAddInsertReadLock node:%d success:%x", ((Node*)node)->id, node->insertLock);
        return TRUE;
    } else{
       vmlog(RemoveLog,"NodeTryAddInsertReadLock node:%d failed:%x, %d", ((Node*)node)->id, node->insertLock, res);
        return FALSE;
    }
}
void NodeRmInsertReadLock(Node* node, int threadId){
    pthread_rwlock_unlock(&node->insertLock);
    vmlog(RemoveLog,"NodeRmInsertReadLock node:%d success :%x", ((Node*)node)->id, node->insertLock);
}

void NodeRmInsertReadLockNoLog(Node* node, int threadId){
    //    int tid = getThreadId();
//        vmlog(InsertLog,"NodeRmInsertReadLock node:%d", ((Node*)node)->id);
    pthread_rwlock_unlock(&node->insertLock);
//    vmlog(RemoveLog,"NodeRmInsertReadLock node:%d success", ((Node*)node)->id);
}


void NodeAddRemoveReadInsertWriteLock(Node* node, int threadId){
    NodeAddRemoveReadLock(node, threadId);
    NodeAddInsertWriteLock(node);

}
void NodeRmRemoveReadInsertWriteLock(Node* node, int threadId){
//    vmlog(InsertLog,"NodeRmRemoveReadInsertWriteLock node:%d", ((Node*)node)->id);
    pthread_rwlock_unlock(&node->insertLock);
    __sync_and_and_fetch(&node->removeLock, ~(1<<(threadId + RemoveReadOff)));
//    vmlog(RemoveLog,"NodeRmRemoveReadInsertWriteLock node:%d success :%x, %x", ((Node*)node)->id, node->insertLock, node->removeLock);

}


void NodeRmRemoveReadLock(Node* node, int threadId){
    if(node == NULL){
        return;
    }
    __sync_and_and_fetch(&node->removeLock, ~(1<<(threadId + RemoveReadOff)));
//    vmlog(RemoveLog,"NodeRmRemoveReadLock node:%d success :%x", ((Node*)node)->id, node->removeLock);
}

inline void NodeAddRemoveReadLock(Node* node, int threadId){
//    vmlog(RemoveLog,"NodeAddRemoveReadLock node:%d ", ((Node*)node)->id);
    //    int tid = getThreadId();
    int try = 0;
    while (1){
        LockType  flag = node->removeLock;
        if((flag & 1) == 0){
            LockType target = flag | (1 << (threadId + RemoveReadOff));
            if(__sync_bool_compare_and_swap(&node->removeLock, flag, target)){
//                vmlog(RemoveLog,"NodeAddRemoveReadLock node:%d success :%x", ((Node*)node)->id, node->removeLock);
                return;
            }
        } else{
            try ++;
            if(try%WriteLockTryThreshold == 0){
                usleep(100);
                vmlog(WARN,"NodeAddRemoveReadLock node:%d conflict:%x", ((Node*)node)->id, node->removeLock);
            }
        }
    }
}

void NodeAddRemoveWriteLock(Node* node){
//    vmlog(RemoveLog,"NodeAddRemoveWriteLock node:%d ", ((Node*)node)->id);
    __sync_fetch_and_or (&node->removeLock, 1);
    int try = 0;
    while (1){
        if(__sync_bool_compare_and_swap(&node->removeLock, 1, 3)){
//            vmlog(RemoveLog,"NodeAddRemoveWriteLock node:%d success", ((Node*)node)->id);
            return;
        } else{
            __sync_fetch_and_or (&node->removeLock, 1);
            try ++;
            if(try%WriteLockTryThreshold == 0){
                vmlog(WARN,"NodeAddRemoveWriteLock node:%d conflict:%x", ((Node*)node)->id, node->removeLock);
                usleep(100);
                if(try % (10 * WriteLockTryThreshold) == 0){
                    vmlog(ERROR,"NodeAddRemoveWriteLock node:%d conflict:%x", ((Node*)node)->id, node->removeLock);
                }
            }
        }
    }
}

BOOL NodeAddRemoveWriteLockNoWait(Node* node){
//    vmlog(RemoveLog,"NodeAddRemoveWriteLockNoWait node:%d ", ((Node*)node)->id);
    while (1){
        if(__sync_bool_compare_and_swap(&node->removeLock, 0, 3)){
//            vmlog(RemoveLog,"NodeAddRemoveWriteLockNoWait node:%d success", ((Node*)node)->id);
            return TRUE;
        } else{
//            vmlog(RemoveLog,"NodeAddRemoveWriteLockNoWait node:%d conflict:%x", ((Node*)node)->id, node->removeLock);
            return FALSE;
        }
    }
}


void NodeRmRemoveWriteLock(Node* node){
    __sync_bool_compare_and_swap(&node->removeLock, 3, 0);
//    vmlog(RemoveLog,"NodeRmRemoveWriteLock node:%d suceess", ((Node*)node)->id);
}

BOOL NodeTryAddRemoveWriteLock(Node* node){
//    vmlog(RemoveLog,"NodeTryAddRemoveWriteLock node:%d", ((Node*)node)->id);
    int try = 0;
    if((node->removeLock & 1) == 1){
//        vmlog(RemoveLog,"NodeTryAddRemoveWriteLock node:%d got by another thread :%x", ((Node*)node)->id, node->removeLock);
        return FALSE;
    } else{
        __sync_fetch_and_or (&node->removeLock, 1);
    }
    while (1){
        if(__sync_bool_compare_and_swap(&node->removeLock, 1, 3)){
//            vmlog(RemoveLog,"NodeTryAddRemoveWriteLock node:%d success", ((Node*)node)->id);
            return TRUE;
        }else{
            try ++;
            __sync_fetch_and_or (&node->removeLock, 1);
            if(try%WriteLockTryThreshold == 0){
                vmlog(WARN,"NodeTryAddRemoveWriteLock node:%d conflict:%x", ((Node*)node)->id, node->removeLock);
                usleep(100);
                if(try % (10 * WriteLockTryThreshold) == 0){
                    vmlog(ERROR,"NodeTryAddRemoveWriteLock node:%d conflict:%x", ((Node*)node)->id, node->removeLock);
                }
            }
        }
    }
//    vmlog(RemoveLog,"NodeTryAddRemoveWriteLock node:%d failed:%x", ((Node*)node)->id, node->removeLock);
    return FALSE;
}

int getThreadId(){
    return 1;
//    int tid = (int )pthread_getspecific(threadId);
//    vmlog(MiXLog, "getThreadId:%d", tid);
//    return tid;
}

BoundKey NodeGetMaxValue(Node* node){
//    BoundKey key = __sync_fetch_and_add (&node->maxValue, 0);
    return node->maxValue;
}

BoundKey NodeGetMinValue(Node* node){
//    BoundKey key = __sync_fetch_and_add (&node->minValue, 0);
    return node->minValue;
}

void NodeModidyRightLeft(Node* node){
    if(node->right != NULL){
        NodeAddInsertWriteLock(node->right);
        node->right->left = node;
        NodeRmInsertWriteLock(node->right);
    }
}