//
// Created by jyh_2 on 2021/5/14.
//

#ifndef QTREE_QTREE_H
#define QTREE_QTREE_H

#include "../query/QueryMeta.h"
#include "../query/QueryRange.h"
#include "../holder/RangeHolder.h"
#include "../Tool/ArrayList.h"
#define maxDepth 16
#define Border  65

#define stackPop(stack, index)  stack[ --index]
#define stackPush(stack, index, elem)  stack[index ++] = elem
#define stackEmpty(stack, index)  (index==0)

#define NodeIsLeaf(node)  (((Node*)node)->id >= 0)

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

#define KeyType QueryRange
#define ValueType   QueryMeta
#define batchSize    2
#define MaxBatchCount (Border/2 - 1)
typedef struct QTree {
    Node *root;
    int elements;
    int maxNodeID;
    int stackNodesIndex;
    int stackSlotsIndex;
    size_t leafSplitCount;
    size_t internalSplitCount;
    size_t funcCount;
    size_t whileCount;
    InternalNode* stackNodes[maxDepth];
    int          stackSlots[maxDepth];
    int     batchCount[batchSize];
    BoundKey batchSearchKey[batchSize];
    KeyType batchKey[batchSize][MaxBatchCount];
    ValueType* batchValue[batchSize][MaxBatchCount];
    pthread_rwlock_t batchRwlock[batchSize];
    int     batchIndex;
}QTree;

typedef struct Node{
    pthread_rwlock_t rwlock;
    int id ;
    int allocated ;
    BoundKey maxValue ;
    BoundKey minValue;
    QTree* tree;
    KeyType  keys[Border];  // array of key
}Node;


struct LeafNode {
    Node  node;
    ValueType* values[Border];
};

 struct InternalNode {
    Node node;
    Node* childs[Border + 1];
};


void QTreeConstructor(QTree* qTree,  int BOrder);
void QTreeDestroy(QTree* qTree);
void printQTree( QTree* qTree);
int  QTreeAllocNode(QTree* qTree, BOOL isLeaf);
void QTreeMakeNewRoot(QTree* qTree, Node* splitedNode);
LeafNode* QTreeFindLeafNode(QTree* qTree, KeyType * key);
void QTreePut(QTree* qTree, KeyType * key, ValueType * value);
void QTreeFindAndRemoveRelatedQueries(QTree* qTree, int attribute, Arraylist* removedQuery);
void QTreePutBatch(QTree* qTree, QueryRange key[], QueryMeta* value[], int bachSize);
void QTreeCheckBatch(QTree* qTree, int attribute, Arraylist* removedQuery);

void NodeCheckTree(Node* node);
void NodeConstructor(Node* node, QTree *tree);
void NodeDestroy(Node* node);
BOOL NodeIsUnderFlow(Node* node);
BOOL NodeCanMerge( Node* node, Node* other) ;
BOOL NodeIsFull(Node* node);
int NodeFindSlotByKey( Node* node, KeyType* searchKey);
void setSearchKey(Node* node, KeyType * key);
void printNode(Node* node);
Node* NodeSplit(Node* node);
int NodeGetHeight(Node* node);
void NodeResetId(Node* node);
void NodeMerge(Node* node, InternalNode* nodeParent, int slot,
               Node* nodeFROM);
KeyType NodeSplitShiftKeysLeft(Node* node);



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
BoundKey  InternalNodeRemove(InternalNode* node, int slot);
void InternalNodeMerge(Node* node, InternalNode* nodeParent, int slot, Node* nodeFROMx);
void printInternalNode(InternalNode* internalNode);

void quickSelect(KeyType arr[], int k, int s, int e);
void swap(KeyType arr[], int a, int b);

#endif //QTREE_QTREE_H
