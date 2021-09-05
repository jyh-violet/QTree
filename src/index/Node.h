//
// Created by jyh_2 on 2021/5/16.
//

#ifndef QTREE_NODE_H
#define QTREE_NODE_H

#include "../common.h"
#include "QTree.h"
#include "LeafNode.h"

template <typename K, typename V>
class QTree;

template <typename K, typename V>
class LeafNode;

template <typename K, typename V>
class InternalNode;


template <typename K, typename V>
class Node{
public:
    int id = 0;
    int allocated = 0;
    K* maxValue = NULL;
    K* minValue = NULL;
    K**  keys;
    QTree<K,V>* tree;

    Node(QTree<K,V> *tree){
        this->tree = tree;
        this->keys = new K*[tree->Border];

    }
    ~Node(){
        delete[] keys;
    }

    bool isUnderFlow();
    bool canMerge( Node<K, V>* other);

    bool isFull();

    int findSlotByKey( K* searchKey);

    void setSearchKey(K* key);

    virtual Node<K, V>* split() = 0;


    virtual K* remove(int slot) = 0;

    virtual bool isLeaf() = 0;


    virtual void allocId() = 0;

    virtual  K* splitShiftKeysLeft() = 0;

    virtual int getId() = 0;

    virtual int getHeight() = 0;
    virtual void resetId() = 0;

    virtual void merge( InternalNode<K, V>* nodeParent, int slot,
                Node<K, V>* nodeFROM) = 0;

    template<class KK, class  VV> friend ostream& operator<< (ostream &out, Node<KK,VV>* node);

};



#endif //QTREE_NODE_H
