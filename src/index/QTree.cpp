//
// Created by workshop on 8/24/2021.
//
#include "QTree.h"

template class QTree<QueryRange, QueryMeta>;

template <typename K, typename V>
int QTree<K,V>::allocNode(bool isLeaf){
    int id = this->maxNodeID ++;
    return isLeaf ? id : -id;
}


template <typename K, typename V>
Node<K,V>* QTree<K,V>::put(K* key, V* value){
    if(key == NULL || value == NULL){
        return NULL;
    }

    Node<K, V>* splitedNode = NULL;

    LeafNode<K, V>* nodeLeaf = this->findLeafNode(key, this->stackNodes, this->stackSlots);

    if (nodeLeaf == NULL) {
        stringstream  ss("");
        while (!stackNodes->empty()) {
            ss << "\n" << stackNodes->top()->getId();
            stackNodes->pop();
        }
        cout << ss.str() << endl;
        exit(-1);
    }
    //
    // Find in leaf node for key
    int slot = nodeLeaf->findSlotByKey(key);

    slot = (slot >= 0)?(slot + 1):((-slot) - 1);

    nodeLeaf->add(slot, key, value);
    splitedNode = (nodeLeaf->isFull() ? nodeLeaf->split() : NULL);
    // Iterate back over nodes checking overflow / splitting
    while (!stackNodes->empty()) {
        //        cout << slot << endl;
        InternalNode<K, V>* node = stackNodes->top();
        stackNodes->pop();
        if(node->maxValue == NULL || key->MaxGE(node->maxValue) ){
            node->maxValue = key;
        }
        if(node->minValue == NULL || node->minValue->MinGT(key) ){
            node->minValue = key;
        }
        slot = stackSlots->top();
        stackSlots->pop();
        //            System.out.println(key + ", "  + otherBound + "," + node.id + ", "  + node.keys[0] + "," + slot);
        if (splitedNode != NULL) {
            // split occurred in previous phase, splitedNode is new child
            K* childKey = splitedNode->splitShiftKeysLeft();

            node->add(slot, childKey, splitedNode);

        }

        if(node->isFull()){
            if(node->isLeaf()){
                splitedNode = ((LeafNode<K,V>*)node)->split();
            }else{
                splitedNode = ((InternalNode<K,V>*)node)->split();
            }

        }else{
            splitedNode = NULL;
        }
        //    splitedNode = (node->isFull() ? node->split() : NULL);
    }



    elements++;
    if (splitedNode != NULL) {   // root was split, make new root
        makeNewRoot(splitedNode);
    }
    return splitedNode;

}

template<typename K, typename V>
void QTree<K, V>::makeNewRoot(Node<K, V>* splitedNode){
    InternalNode<K, V>* nodeRootNew = new InternalNode<K, V>(this);
    nodeRootNew->allocId();

    K* newkey = splitedNode->splitShiftKeysLeft();

    nodeRootNew->childs[0] = root;
    nodeRootNew->keys[0] = newkey;
    nodeRootNew->childs[1] = splitedNode;
    nodeRootNew->allocated++;
    nodeRootNew->resetMaxValue();
    nodeRootNew->resetMinValue();
    root = nodeRootNew;
}

template<typename K, typename V>
LeafNode<K, V>* QTree<K, V>::findLeafNode(K* key, stack<InternalNode<K,V>*>* stackNodes, stack<int>* stackSlots) {
    Node<K, V>* node = root;
    int slot = 0;
    while (!node->isLeaf()) {
        InternalNode<K, V> *nodeInternal = (InternalNode<K, V>*) node;
        nodeInternal->setSearchKey(key);
        slot = node->findSlotByKey(key);

        slot = ((slot < 0) ? (-slot) - 1 : slot + 1);

        stackNodes->push(nodeInternal);
        stackSlots->push(slot);

        node =nodeInternal->childs[slot];
        if (node == NULL) {
            cout << "ERROR childs[" << slot <<  "] in node=" << nodeInternal->getId() << endl;
            exit(-1);
        }
    }
    return (node->isLeaf() ? (LeafNode<K, V>*) node : NULL);
}

template<typename K, typename V>
void QTree<K, V>::findAndRemoveRelatedQueries(int attribute, list<QueryMeta*>* removedQuery){
    Node<K, V>* node = this->root;
    int slot = 0;
    K* key = new K(attribute, attribute, true, true);
    K *removedMax, *removedMin;
    bool resetMax = false;
    bool resetMin = false;
    while (true) {
        while (!node->isLeaf()){
            bool getNode = false;
            InternalNode<K, V>* nodeInternal = (InternalNode<K, V>*) node;
            for(slot = 0; slot <= nodeInternal->allocated; slot ++){
                if(nodeInternal->childs[slot]->maxValue->MaxGE(key)){
                    node = nodeInternal->childs[slot];
                    stackNodes->push(nodeInternal);
                    stackSlots->push(slot);
                    getNode = true;
                    break;
                }
                if(slot < nodeInternal->allocated && nodeInternal->childs[slot + 1]->minValue ->MinGT(key)){
                    break;
                }
            }
            //                System.out.println("getNode:" + getNode + ", node:" + node);
            if(!getNode){
                bool getAnother = false;
                while (!stackNodes->empty()){
                    node = stackNodes->top();
                    stackNodes->pop();
                    slot = stackSlots->top();
                    stackSlots->pop();
                    if(slot >= node->allocated  || ((InternalNode<K, V>*) node)->childs[slot + 1]->minValue->MinGT(key)){
                        for(int i = 0; i < slot; i ++ ){
                            ((InternalNode<K, V>*) node)->checkUnderflowWithRight(i);
                        }
                        if(node->maxValue == removedMax){
                            ((InternalNode<K, V>*) node)->resetMaxValue();
                        }
                        if(node->minValue == removedMin){
                            ((InternalNode<K, V>*) node)->resetMinValue();
                        }
                    }else {
                        InternalNode<K, V>* internalNode = (InternalNode<K, V>*) node;
                        node = internalNode->childs[slot + 1];
                        stackNodes->push(internalNode);
                        stackSlots->push(slot + 1);
                        //                            System.out.println("another node:" + node);
                        slot = 0;
                        getAnother = true;
                        break;
                    }
                }
                if(!getAnother){
                    break;
                }
            }
        }
        if(node->isLeaf()){
            int j = 0;
            LeafNode<K, V>* leafNode = (LeafNode<K, V>*) node;
            //                System.out.println("getLeafNode:" + leafNode);
            resetMax = false;
            resetMin = false;
            for(int i = 0; i < leafNode->allocated ; i ++){
                //                    System.out.println("query:" + leafNode.values[i]);
                if(leafNode->values[i]->cover(attribute)){
                    if(leafNode->maxValue == leafNode->keys[i]){
                        resetMax = true;
                        removedMax = (leafNode->maxValue);
                    }
                    if(leafNode->minValue == leafNode->keys[i]){
                        resetMin = true;
                        removedMin = leafNode->minValue;
                    }
                    removedQuery->push_back(leafNode->values[i]);
                    elements --;
                }else {
                    leafNode->keys[j] = leafNode->keys[i];
                    leafNode->values[j ++] = leafNode->values[i];
                }
            }
            leafNode->allocated = j;
            if(resetMax){
                leafNode->resetMaxValue();
            }else{
                removedMax = NULL;
            }
            if(resetMin){
                leafNode->resetMinValue();
            }else{
                removedMin = NULL;
            }
            if(stackNodes->empty()){
                break;
            }
            bool getAnother = false;
            while (!stackNodes->empty()){
                node = stackNodes->top();
                stackNodes->pop();
                slot = stackSlots->top();
                stackSlots->pop();

                if(slot >= node->allocated  || ((InternalNode<K, V>*) node)->childs[slot + 1]->minValue->MinGT(key)){
                    for(int i = 0; i < slot; i ++ ){
                        ((InternalNode<K, V>*) node)->checkUnderflowWithRight(i);
                    }
                    if(node->maxValue == removedMax){
                        ((InternalNode<K, V>*) node)->resetMaxValue();
                    }
                    if(node->minValue == removedMin){
                        ((InternalNode<K, V>*) node)->resetMinValue();
                    }

                }else {
                    InternalNode<K,V>* internalNode = (InternalNode<K, V>*) node;
                    node = internalNode->childs[slot + 1];
                    stackNodes->push(internalNode);
                    stackSlots->push(slot + 1);
                    slot = 0;
                    getAnother = true;
                    break;
                }
            }
            if(!getAnother){
                break;
            }
        }else {
            break;
        }
    }
}


