//
// Created by jyh_2 on 2021/5/14.
//

#ifndef QTREE_QTREE_H
#define QTREE_QTREE_H

#include "../query/QueryMeta.h"
#include "../query/QueryRange.h"
#include "../holder/RangeHolder.h"
#include "../Tool/ArrayList.h"
#define maxDepth 32
#define Border  5

#define stackPop(stack, index)  stack[ --index]
#define stackPush(stack, index, elem)  stack[index ++] = elem
#define stackEmpty(stack, index)  (index==0)

#define NodeIsLeaf(node)  (((Node*)node)->id >= 0)

int checkQueryMeta;
int setKeyCount;

extern int maxValue;
extern int removePoint;
extern int Qid;

typedef struct InternalNode InternalNode;
typedef struct LeafNode LeafNode;
typedef struct Node Node;

#define RemovedQueueSize 8
BoundKey RemovedKey[RemovedQueueSize];
u_int32_t clockFlag;
int clockIndex;

int printQTreelog;
int useBFPRT;

OptimizationType optimizationType;

#define KeyType QueryRange
#define ValueType   QueryMeta
#define batchMissThreshold  5
#define MaxBatchCount (Border/2 - 1)

//extern pthread_key_t threadId;

typedef struct NodesStack{
    InternalNode*   stackNodes[maxDepth];
    int             stackNodesIndex;
}NodesStack;

typedef struct IntStack{
    int          stackSlots[maxDepth];
    int          stackSlotsIndex;
}IntStack;

typedef struct QueryData{
    KeyType key;
    ValueType *value;
}QueryData;

typedef struct QTree {
    _Atomic int elements;
    _Atomic int maxNodeID;
    _Atomic int     batchCount;
    _Atomic int     batchMissCount;
    BoundKey batchSearchKey;
    QueryData batch[MaxBatchCount];
    _Atomic size_t leafSplitCount;
    _Atomic size_t internalSplitCount;
    _Atomic size_t funcCount;
    _Atomic size_t whileCount;
    Node *root;
    pthread_spinlock_t removeLock;
}QTree;


#define NodeIsValid(node)  (((Node*)node)->allocated >= 0)

typedef struct Node{
    pthread_spinlock_t lock; // work as write lock
    _Atomic int read; // work as the read lock
    u_int64_t insertLock; // work as the read lock
    int id ;
    int allocated ;
    BoundKey maxValue ;
    BoundKey minValue;
    BoundKey nextNodeMin;
    Node *right;
    QTree* tree;
}Node;


struct LeafNode {
    Node  node;
    QueryData data[Border];
};

struct InternalNode {
    Node node;
    pthread_rwlock_t removeLock;
    KeyType  keys[Border];  // array of key
    Node* childs[Border + 1];
};


void QTreeConstructor(QTree* qTree,  int BOrder);
void QTreeDestroy(QTree* qTree);
void printQTree( QTree* qTree);
int  QTreeAllocNode(QTree* qTree, BOOL isLeaf);
void QTreeMakeNewRoot(QTree* qTree, Node* splitedNode);
LeafNode* QTreeFindLeafNode(QTree* qTree, KeyType * key, NodesStack* nodesStack, IntStack* slotStack, BoundKey min, BoundKey max, int threadId);
void QTreePut(QTree* qTree, KeyType * key, ValueType * value, int threadId);
void QTreeFindAndRemoveRelatedQueries(QTree* qTree, int attribute, Arraylist* removedQuery);
void QTreePutBatch(QTree* qTree, QueryData * batch, int batchCount, int threadId);
void QTreeCheckBatch(QTree* qTree, int attribute, Arraylist* removedQuery);
void QTreePutOne(QTree* qTree, QueryRange* key, QueryMeta* value, int threadId);
Node* checkInternalNode(QTree* qTree, InternalNode* nodeInternal,  KeyType* key, NodesStack *nodesStack, IntStack* slotStack);
void checkLeafNode(QTree* qTree, LeafNode* leafNode, BoundKey* removedMax, BoundKey* removedMin, BoundKey attribute, Arraylist* removedQuery);
Node* getAnotherNode(QTree* qTree, KeyType* key, BoundKey *removedMax, BoundKey *removedMin, Arraylist* removedQuery, NodesStack *nodesStack, IntStack* slotStack);
BOOL QTreeCheckMaxMin(QTree* qTree);
void printQTreeStatistics(QTree * qTree);
BOOL CheckLeafNodeCover(LeafNode * leafNode, int i,  BoundKey attribute);
BOOL QTreeCheckKey(QTree* qTree);
void QTreeResetStatistics(QTree* qTree);
BOOL QTreeAddLockForFindLeaf(Node* node, int threadId);
void QTreeRmLockForFindLeaf(Node* node, int threadId);
void QTreeModifyNodeMaxMin(Node* node, BoundKey min, BoundKey max);
Node* QTreeTravelRightLink(Node* node, KeyType * key, int threadId);

void NodeCheckTree(Node* node);
void NodeConstructor(Node* node, QTree *tree);
void NodeDestroy(Node* node);
BOOL NodeIsUnderFlow(Node* node);
BOOL NodeCanMerge( Node* node, Node* other) ;
BOOL NodeIsFull(Node* node);
void setSearchKey(Node* node, KeyType * key);
void printNode(Node* node);
Node* NodeSplit(Node* node);
int NodeGetHeight(Node* node);
void NodeResetId(Node* node);
void NodeMerge(Node* node, InternalNode* nodeParent, int slot,
               Node* nodeFROM);
KeyType NodeSplitShiftKeysLeft(Node* node);
BOOL NodeCheckMaxMin(Node * node);
BOOL NodeCheckKey(Node * node);
BOOL NodeCheckLink(Node* node);
void NodeAddWriteLock(Node* node);
void NodeRmWriteLock(Node* node);
void NodeAddRWLock(Node* node);
void NodeRmRWLock(Node* node);
void NodeRmReadLock(Node* node);
void NodeAddReadLock(Node* node);
void NodeAddInsertReadLock(Node* node, int threadId);
void NodeRmInsertReadLock(Node* node, int threadId);
void NodeAddInsertRWLock(Node* node);
void NodeRmInsertRWLock(Node* node);
BOOL NodeTryAddWriteLock(Node* node);
BOOL NodeTryAddInsertReadLock(Node* node, int threadId);

void LeafNodeConstructor(LeafNode* leafNode, QTree *tree);
void LeafNodeDestroy(LeafNode* leafNode);
void LeafNodeMerge(LeafNode* leafNode, InternalNode* nodeParent, int slot,
                   Node* nodeFROMx);
void * LeafNodeRemove(LeafNode* leafNode, int slot);
BOOL LeafNodeAdd(LeafNode* leafNode, int slot, KeyType * newKey, ValueType * newValue);
BOOL LeafNodeAddLast(LeafNode* leafNode, KeyType * newKey, ValueType * newValue);
void LeafNodeAllocId(LeafNode* leafNode);
void LeafNodeResetMaxValue(LeafNode* node);
void LeafNodeResetMinValue(LeafNode* node);
Node* LeafNodeSplit(LeafNode* leafNode);
KeyType  LeafNodeSplitShiftKeysLeft(LeafNode* node);
int LeafNodeGetId(LeafNode* node) ;
int LeafNodeGetHeight(LeafNode* node);
void LeafNodeResetId(LeafNode* node);
void printLeafNode(LeafNode* leafNode);
void LeafNodeResetMinKey(LeafNode* leafNode);
BOOL LeafNodeCheckMaxMin(LeafNode * leafNode);
Node* LeafNodeSplit_NoSort(LeafNode* leafNode) ;
Node* LeafNodeSplit_Sort(LeafNode* leafNode);
BOOL LeafNodeCheckKey(LeafNode * leafNode);
BOOL LeafNodeAddBatch(LeafNode* leafNode, int slot, QueryData batch[], int batchCount, BoundKey *min, BoundKey* max);
int LeafNodeFindSlotByKey( LeafNode * node, KeyType* searchKey) ;
BOOL LeafNodeCheckLink(LeafNode* node);

void InternalNodeConstructor(InternalNode* internalNode, QTree* qTree);
void InternalNodeDestroy(InternalNode* internalNode);
BOOL InternalNodeAdd(InternalNode* node, int slot, KeyType * newKey, Node* child);
void InternalNodeResetMaxValue(InternalNode* node);
void InternalNodeResetMinValue(InternalNode* node);
void InternalNodeAllocId(InternalNode* node);
Node* InternalNodeSplit(InternalNode* node);
KeyType  InternalNodeSplitShiftKeysLeft(InternalNode* node);
int InternalNodeGetId(InternalNode* node);
int InternalNodeGetHeight(InternalNode* node);
void InternalNodeResetId(InternalNode* node);
BOOL InternalNodeCheckUnderflowWithRight(InternalNode* node, int slot);
void  InternalNodeRemove(InternalNode* node, int slot);
void InternalNodeMerge(Node* node, InternalNode* nodeParent, int slot, Node* nodeFROMx);
void printInternalNode(InternalNode* internalNode);
BOOL InternalNodeCheckMaxMin(InternalNode * internalNode);
BOOL InternalNodeCheckKey(InternalNode * internalNode);
int InternalNodeFindSlotByKey( InternalNode * node, KeyType* searchKey) ;
int InternalNodeFindSlotByNextMin( InternalNode* node, BoundKey nextMin);
BOOL InternalNodeCheckLink(InternalNode * node);
void InternalNodeAddRemoveLock(InternalNode* internalNode);
void InternalNodeRmRemoveLock(InternalNode* internalNode);

void quickSelect(QueryData data[], int k, int s, int e);
//void quickSelect(KeyType arr[], int k, int s, int e);
void swap(KeyType arr[], int a, int b);

int getThreadId();

BoundKey NodeGetMaxValue(Node* node);
BoundKey NodeGetMinValue(Node* node);

#endif //QTREE_QTREE_H
