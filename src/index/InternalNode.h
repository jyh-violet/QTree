//
// Created by jyh_2 on 2021/5/18.
//

#ifndef QTREE_INTERNALNODE_H
#define QTREE_INTERNALNODE_H
#include "Node.h"

template <typename K, typename V>
class Node;
template <typename K, typename V>
class QTree;

template <typename K, typename V>
class InternalNode : public Node<K, V>{
public:
    Node<K,V>** childs;
    InternalNode(QTree<K,V> *tree) : Node<K,V>(tree){
        childs = new Node<K,V>*[tree->Border + 1];
    }
    ~InternalNode(){
        delete[] childs;
    }

    void resetMaxValue();

    void resetMinValue();


    bool add(int slot, K* newKey, Node<K,V>* child);

    bool checkUnderflowWithRight(int slot);

    K* remove(int slot) override;

    bool isLeaf() override;

    Node<K, V>* split() override;

    void allocId() override;

    K* splitShiftKeysLeft() override;

    int getId() override;

    int getHeight() override;
    void merge( InternalNode<K, V>* nodeParent, int slot,
               Node<K, V>* nodeFROM) override;

    template<class KK, class  VV> friend ostream& operator<< (ostream &out, InternalNode<KK,VV>* internalNode);
};


template<typename K, typename V>
bool InternalNode<K, V>::isLeaf() {
    return false;
}

template<typename K, typename V>
bool InternalNode<K, V>::add(int slot, K* newKey, Node<K,V>* child){
    if (slot < this->allocated) {
        memcpy(this->keys + slot + 1, this->keys + slot, (this->allocated - slot) * sizeof(K*));
        memcpy(this->childs + slot + 2, this->childs + slot + 1, (this->allocated - slot) * sizeof(Node<K,V>*));
    }
    this->allocated++;
    this->keys[slot] = newKey;
    this->childs[slot + 1] = child;
    return true;
}


template<typename K, typename V>
Node<K, V>* InternalNode<K, V>::split() {
    InternalNode<K, V>* newHigh = new InternalNode<K,V>(this->tree);
    newHigh->allocId();
    // int j = ((allocated >> 1) | (allocated & 1)); // dividir por dos y sumar el resto (0 o 1)
    int j = (this->allocated >> 1); // dividir por dos (libro)
    int newsize = this->allocated - j;

    // if (log.isDebugEnabled()) log.debug("split j=" + j);
    memcpy(newHigh->keys, this->keys + j,  newsize * sizeof (K**));

    memcpy(newHigh->childs, this->childs + (j+1),  newsize * sizeof (Node<K,V>**));

    for (int i = j; i < j + newsize; i++) {
        this->keys[i] = NULL;
        this->childs[i + 1] = NULL;
    }
    newHigh->allocated = newsize;
    this->allocated -= newsize;
    if(*(this->maxValue) > *(newHigh->keys[0])){
        newHigh->maxValue = this->maxValue;
        this->resetMaxValue();

    }else {
        newHigh->allocated --;
        newHigh->resetMaxValue();
        newHigh->allocated ++;
    }

    if(*(this->minValue) > *(newHigh->keys[0])){
        newHigh->minValue = this->minValue;
        this->resetMinValue();
    }else {
        newHigh->allocated --;
        newHigh->resetMinValue();
        newHigh->allocated ++;
    }
    return newHigh;
}

template<typename K, typename V>
void InternalNode<K, V>::resetMaxValue(){
    this->maxValue = this->childs[0]->maxValue;
    for(int i = 1; i <= this->allocated; i ++){
        if( (childs[i]->maxValue)->MaxGE(this->maxValue)){
            this->maxValue = childs[i]->maxValue;
        }
    }
}

template<typename K, typename V>
void InternalNode<K, V>::resetMinValue(){
    this->minValue = this->childs[0]->minValue;
    for(int i = 1; i <= this->allocated; i ++){
        if((this->minValue)->MinGT (childs[i]->minValue)){
            this->minValue = childs[i]->minValue;
        }
    }
}

template<typename K, typename V>
void InternalNode<K, V>::allocId() {
    this->id = this->tree->allocNode(false);
}

template<typename K, typename V>
K* InternalNode<K, V>::splitShiftKeysLeft() {
    K* removed = this->keys[0];
    memcpy(this->keys, this->keys + 1, (this->allocated - 1) * sizeof(K*));
    this->allocated--;
    this->keys[this->allocated] = NULL;
    this->childs[this->allocated + 1] = NULL;
    return removed;
}

template<typename K, typename V>
int InternalNode<K, V>::getId() {
    return this->id;
}

template<typename K, typename V>
int InternalNode<K, V>::getHeight() {
    return 0;
}

template<typename K, typename V>
bool InternalNode<K, V>::checkUnderflowWithRight(int slot){
    Node<K, V>* nodeLeft = childs[slot];
    while ((slot < this->allocated) && nodeLeft->isUnderFlow()) {
        Node<K, V>* nodeRight = childs[slot + 1];
        if (nodeLeft->canMerge(nodeRight)) {
            nodeLeft->merge(this, slot, nodeRight);
            childs[slot] = nodeLeft;
        } else {
//                nodeLeft.shiftRL(this, slot, nodeRight);
        }

        return true;
    }
    return false;
}

template<typename K, typename V>
void InternalNode<K, V>::merge(InternalNode<K, V>* nodeParent, int slot, Node<K, V>* nodeFROMx) {
    InternalNode<K, V>* nodeFROM = (InternalNode<K, V>*) nodeFROMx;
    InternalNode<K, V>* nodeTO = this;
    int sizeTO = nodeTO->allocated;
    int sizeFROM = nodeFROM->allocated;
    // copy keys from nodeFROM to nodeTO
    memcpy(nodeTO->keys + sizeTO + 1, nodeFROM->keys, sizeFROM * sizeof(K*));
    memcpy(nodeTO->childs + sizeTO + 1, nodeFROM->childs, (sizeFROM + 1) * sizeof(K*));
    // add key to nodeTO
    nodeTO->keys[sizeTO] = nodeParent->keys[slot];
    nodeTO->allocated += sizeFROM + 1; // keys of FROM and key of nodeParent
    if(!nodeTO->maxValue->MaxGE(nodeFROM->maxValue)){
        nodeTO->maxValue = nodeFROM->maxValue;
    }
    if(nodeTO->minValue->MinGT(nodeFROM->minValue)){
        nodeTO->minValue = nodeFROM->minValue;
    }

    // remove key from nodeParent
    nodeParent->remove(slot);
    // Free nodeFROM
    delete nodeFROM;

}

template<typename K, typename V>
K* InternalNode<K, V>::remove( int slot) {
    if (slot < -1) {
        cout << "faking slot=" << slot << " allocated=" << this->allocated << endl;
        return NULL;
    }
    K* removedUpper = childs[slot + 1]->maxValue;

    if(slot == -1){
        memcpy(this->keys, this->keys + 1, (this->allocated - 1) * sizeof (K*));
        memcpy(this->childs, this->childs + 1, (this->allocated) * sizeof (Node<K,V>*));
    }else if (slot < this->allocated) {
        memcpy(this->keys + slot, this->keys + slot + 1, (this->allocated - slot - 1) * sizeof (K*));
        memcpy(this->childs + slot + 1, this->childs + slot + 2, (this->allocated - slot - 1) * sizeof (Node<K,V>*));
    }
    if (this->allocated > 0) {
        this->allocated--;
    }
    this->keys[this->allocated] = NULL;
    childs[this->allocated + 1] = NULL;
    return removedUpper;
}

template<typename K, typename V>
ostream& operator<< (ostream &out, InternalNode<K,V>* internalNode){
    out << "[I" << internalNode->id << "](" << internalNode->allocated << ")(" << maxValue << "){";
    for (int i = 0; i < internalNode->allocated; i++) {
        K* k = internalNode->keys[i];
        if (i == 0) { // left
            out << "C" << i << ":Node" << internalNode->childs[i]->id << "(" << internalNode->childs[i]->maxValue << ")<";
        } else {
            out << "<";
        }
        out << k;
        out << ">C" << i + 1 << ":Node"<< internalNode->childs[i + 1]->id
            <<"(" << internalNode->childs[i + 1]->maxValue << ")";

    }
    out << "}";
    return out;
}

#endif //QTREE_INTERNALNODE_H
