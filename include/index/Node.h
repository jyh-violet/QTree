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

    virtual Node<K, V>* split() = 0;


    virtual K* remove(int slot) = 0;

    virtual bool isLeaf() = 0;


    virtual void allocId() = 0;

    virtual  K* splitShiftKeysLeft() = 0;

    virtual int getId() = 0;

    virtual int getHeight() = 0;

    virtual void merge( InternalNode<K, V>* nodeParent, int slot,
                Node<K, V>* nodeFROM) = 0;

    template<class KK, class  VV> friend ostream& operator<< (ostream &out, Node<KK,VV>* node);

};

template<typename K, typename V>
bool Node<K,V>::isUnderFlow() {
    return (allocated < (tree->Border >> 1));
}

template<typename K, typename V>
bool Node<K,V>::canMerge( Node<K, V>* other) {
    return ((allocated + other->allocated + 1) < tree->Border);
}

template<typename K, typename V>
bool Node<K,V>::isFull(){ // node is full
    return (allocated >= tree->Border);
}

template <typename K, typename V>
int Node<K,V>::findSlotByKey( K* searchKey) {
    // return Arrays.binarySearch(keys, 0, allocated, searchKey);
    if(this->allocated == 0){
        return -1;
    }
    int low = 0;
    int high = this->allocated - 1;

    while (low <= high) {
        int mid = (low + high) >> 1;
        K midVal = *(this->keys[mid]);

        if (midVal < *searchKey) {
            low = mid + 1;
        } else if (midVal > *searchKey) {
            high = mid - 1;
        } else {
            return mid; // key found
        }
    }
    return -(low + 1);  // key not found.
}

template<typename K, typename V>
ostream& operator<< (ostream &out, Node<K,V>* node){
    if(node->isLeaf()){
        out << (LeafNode<K,V>*)node;
    }else{
        out << (InternalNode<K,V>*)node;

    }
    return out;
}


#endif //QTREE_NODE_H
