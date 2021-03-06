//
// Created by jyh_2 on 2021/5/16.
//

#ifndef QTREE_LEAFNODE_H
#define QTREE_LEAFNODE_H


#include "Node.h"

template <typename K, typename V>
class Node;
template <typename K, typename V>
class InternalNode;
template <typename K, typename V>
class QTree;

template <typename K, typename V>
class LeafNode : public Node<K,V> {
public:
    V** values;
    LeafNode(QTree<K,V> *tree) : Node<K,V>(tree){
        values = new V*[tree->Border];
    }
    ~LeafNode(){
        delete[] values;
    }

    bool add(int slot, K* newKey, V* newValue);

    void resetMaxValue();

    K* remove(int slot) override;

    bool isLeaf() override;

    Node<K, V>* split() override;

    void allocId() override;

    K* splitShiftKeysLeft() override;

    int getId() override;

    int getHeight() override;

    void merge( InternalNode<K, V>* nodeParent, int slot,
                Node<K, V>* nodeFROM) override;

    template<class KK, class  VV> friend ostream& operator<< (ostream &out, LeafNode<KK,VV>* leafNode);



};

template<typename K, typename V>
void LeafNode<K, V>::merge( InternalNode<K, V>* nodeParent, int slot,
            Node<K, V>* nodeFROM){

}

template<typename K, typename V>
K* LeafNode<K, V>::remove(int slot) {
    return NULL;
}

template<typename K, typename V>
bool LeafNode<K, V>::add(int slot, K* newKey, V* newValue){
    if (slot < this->allocated) {
        memcpy(this->keys + slot + 1, this->keys + slot, (this->allocated - slot) * sizeof(K*));
        memcpy(this->values + slot + 1, this->values + slot, (this->allocated - slot) * sizeof(V*));
    }
    this->allocated++;
    this->keys[slot] = newKey;
    this->values[slot] = newValue;
    if(this->maxValue == NULL || (newKey->MaxGE(this->maxValue))){
        this->maxValue = newKey;
    }
    return true;
}

template<typename K, typename V>
bool LeafNode<K, V>::isLeaf() {
    return true;
}

template<typename K, typename V>
Node<K, V>* LeafNode<K, V>::split() {
    LeafNode<K, V>* newHigh = new LeafNode<K,V>(this->tree);
    newHigh->allocId();

    int j = this->allocated >> 1; // dividir por dos (libro)
    int newsize = this->allocated - j;
    // if (log.isDebugEnabled()) log.debug("split j=" + j);
    memcpy(newHigh->keys, this->keys + j,  newsize * sizeof (K*));

    memcpy(newHigh->values, this->values + j,  newsize * sizeof (V*));

    for (int i = j; i < j + newsize; i++) {
        this->keys[i] = NULL;
        // clear bound id of the influenced query
        this->values[i] = NULL;
    }
    newHigh->allocated = newsize;
    this->allocated -= newsize;
    if(*(this->maxValue) > *(newHigh->keys[0])){
        newHigh->maxValue = this->maxValue;
        this->resetMaxValue();
    }else {
        newHigh->resetMaxValue();
    }

    return newHigh;
}

template<typename K, typename V>
void LeafNode<K, V>::resetMaxValue() {
    if (this->allocated == 0) {
        return;
    }
    this->maxValue = this->keys[0];
    for (int i = 1; i < this->allocated; i++) {
        if ((this->keys[i])->MaxGE(this->maxValue)) {
            this->maxValue = this->keys[i];
        }
    }
}


template<typename K, typename V>
void LeafNode<K, V>::allocId() {
    this->id = this->tree->allocNode(true);
}

template<typename K, typename V>
K* LeafNode<K, V>::splitShiftKeysLeft() {
    return this->keys[0];
}

template<typename K, typename V>
int LeafNode<K, V>::getId() {
    return this->id;
}

template<typename K, typename V>
int LeafNode<K, V>::getHeight() {
    return 1;
}



template<typename K, typename V>
ostream& operator<< (ostream &out, LeafNode<K,V>* leafNode){
    out << "[L" << leafNode->id << "](" << leafNode->allocated << ")(" << maxValue << "){";
    for (int i = 0; i < leafNode->allocated; i++) {
        K* k = leafNode->keys[i];
        V* v = leafNode->values[i];
        out << k << "Q[" << v->getQueryId() << "]|  ";
    }
    out << "}";
    return out;
}


#endif //QTREE_LEAFNODE_H
