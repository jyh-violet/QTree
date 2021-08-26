//
// Created by workshop on 8/24/2021.
//

#include "Node.h"
template class Node<QueryRange, QueryMeta>;

template<typename K, typename V>
bool Node<K,V>::isUnderFlow() {
    return (allocated <= (tree->Border >> 1));
}

template<typename K, typename V>
bool Node<K,V>::canMerge( Node<K, V>* other) {
    return ((allocated + other->allocated) < tree->Border);
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
void Node<K,V>::setSearchKey(K* key){
//    key->searchKey = (key->lower + key->upper) / 2;
//    key->searchKey = key->lower;
    if((key->lower >= this->maxValue->upper) || (key-> upper <= this->minValue->lower)){
        return;
    }
    int low = this->minValue->lower;
    if(key->MinGT(this->minValue)){
        low = key->lower;
    }
    int high = this->maxValue->upper;
    if(!key->MaxGE(this->maxValue)){
        high = key->upper;
    }
    key->searchKey = (low + high) / 2;
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