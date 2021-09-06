//
// Created by jyh_2 on 2021/5/14.
//

#ifndef QTREE_QTREE_H
#define QTREE_QTREE_H

#include "../query/QueryMeta.h"
#include "../query/QueryRange.h"
#include "../holder/RangeHolder.h"
#include "../Tool/ArrayList.h"
#define maxDepth 100
#define Border  33

#define pop(stack, index)  stack[ --index]
#define push(stack, index, elem)  stack[index ++] = elem
#define empty(stack, index)  (index==0)

extern int maxValue;
extern int Qid;

int leafSplitCount;
int internalSplitCount;
int funcCount;
long funcTime;

typedef struct InternalNode InternalNode;
typedef struct LeafNode LeafNode;
typedef struct Node Node;

#define KeyType QueryRange
#define ValueType   QueryMeta

typedef struct QTree {
    Node *root;
    int elements;
    int maxNodeID;
    int stackNodesIndex;
    int stackSlotsIndex;
    InternalNode* stackNodes[maxDepth];
    int          stackSlots[maxDepth];



}QTree;

typedef struct Node{
    bool isLeaf;
    int id ;
    int allocated ;
    KeyType* maxValue ;  //  Key*
    KeyType * minValue; // Key *
    KeyType *  keys[Border];  // array of key*
    QTree* tree;

}Node;


 struct LeafNode {
    Node  node;
    ValueType * values[Border];
};
 struct InternalNode {
    Node node;
    Node* childs[Border + 1];
};


void QTreeConstructor(QTree* qTree,  int BOrder);
void QTreeDestroy(QTree* qTree);
void printQTree( QTree* qTree);
int QTreeAllocNode(QTree* qTree, bool isLeaf);
void QTreeMakeNewRoot(QTree* qTree, Node* splitedNode);
LeafNode* QTreeFindLeafNode(QTree* qTree, KeyType * key);
Node* QTreePut(QTree* qTree, KeyType * key, ValueType * value);
void QTreeFindAndRemoveRelatedQueries(QTree* qTree, int attribute, Arraylist* removedQuery);




void NodeConstructor(Node* node, QTree *tree);
void NodeDestroy(Node* node);
bool NodeIsUnderFlow(Node* node);
bool NodeCanMerge( Node* node, Node* other) ;
bool NodeIsFull(Node* node);
int NodeFindSlotByKey( Node* node, KeyType* searchKey);
void setSearchKey(Node* node, KeyType * key);
void printNode(Node* node);
Node* NodeSplit(Node* node);
int NodeGetHeight(Node* node);
void NodeResetId(Node* node);
void NodeMerge(Node* this, InternalNode* nodeParent, int slot,
               Node* nodeFROM);
void * NodeSplitShiftKeysLeft(Node* node);



void LeafNodeConstructor(LeafNode* leafNode, QTree *tree);
void LeafNodeDestroy(LeafNode* leafNode);
void LeafNodeMerge(LeafNode* leafNode, InternalNode* nodeParent, int slot,
                   Node* nodeFROMx);
void * LeafNodeRemove(LeafNode* leafNode, int slot);
bool LeafNodeAdd(LeafNode* leafNode, int slot, KeyType * newKey, ValueType * newValue);
void LeafNodeAllocId(LeafNode* leafNode);
void LeafNodeResetMaxValue(LeafNode* node);
void LeafNodeResetMinValue(LeafNode* node);
Node* LeafNodeSplit(LeafNode* leafNode);
KeyType * LeafNodeSplitShiftKeysLeft(LeafNode* this);
int LeafNodeGetId(LeafNode* node) ;
int LeafNodeGetHeight(LeafNode* node);
void LeafNodeResetId(LeafNode* node);
void printLeafNode(LeafNode* leafNode);





void InternalNodeConstructor(InternalNode* internalNode, QTree* qTree);
void InternalNodeDestroy(InternalNode* internalNode);
bool InternalNodeAdd(InternalNode* this, int slot, KeyType * newKey, Node* child);
void InternalNodeResetMaxValue(InternalNode* this);
void InternalNodeResetMinValue(InternalNode* this);
void InternalNodeAllocId(InternalNode* this);
Node* InternalNodeSplit(InternalNode* this);
KeyType * InternalNodeSplitShiftKeysLeft(InternalNode* this);
int InternalNodeGetId(InternalNode* this);
int InternalNodeGetHeight(InternalNode* this);
void InternalNodeResetId(InternalNode* this);
bool InternalNodeCheckUnderflowWithRight(InternalNode* this, int slot);
KeyType * InternalNodeRemove(InternalNode* this, int slot);
void InternalNodeMerge(Node* this, InternalNode* nodeParent, int slot, Node* nodeFROMx);
void printInternalNode(InternalNode* internalNode);



#endif //QTREE_QTREE_H
