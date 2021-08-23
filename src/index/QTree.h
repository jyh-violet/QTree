//
// Created by jyh_2 on 2021/5/14.
//

#ifndef QTREE_QTREE_H
#define QTREE_QTREE_H


#include "Node.h"
#include "LeafNode.h"
#include "InternalNode.h"
#include "../query/QueryMeta.h"
#include "../query/QueryRange.h"
#include "holder/RangeHolder.h"
#include <stack>
#include <list>


template <typename K, typename V>
class Node;
template <typename K, typename V>
class LeafNode;
template <typename K, typename V>
class InternalNode;

template <typename K, typename V>
class QTree {
public:
    int Border;
    Node<K, V> *root;
    Node<K, V> *low;
    Node<K, V> *high;
    int elements = 0;
    int maxNodeID = 0;
    stack<InternalNode<K, V>*>* stackNodes;
    stack<int>* stackSlots;

    QTree(int BOrder){
        BOrder += 1 - (BOrder % 2);
        this->Border = BOrder + 1;
        this->root = new LeafNode<K,V>(this);
        this->root->allocId();
        this->elements = 0;
        this->low = this->high = this->root;
        this->stackNodes = new  stack<InternalNode<K, V>*>();
        this->stackSlots = new stack<int>();
    }
    ~QTree(){
        delete this->root;
        delete stackNodes;
        delete stackSlots;
    }

    int allocNode(bool isLeaf);

    LeafNode<K, V>* findLeafNode(K* key, stack<InternalNode<K, V>*>* stackNodes, stack<int>* stackSlots);

    Node<K, V> *put(K *key, V *value);

    void makeNewRoot(Node<K, V>* splitedNode);

    void findAndRemoveRelatedQueries(int attribute, list<QueryMeta*>* removedQuery);

    template<class KK, class  VV> friend ostream& operator<< (ostream &out, QTree<KK,VV>* qTree);

};


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
    root = nodeRootNew;
}

template<typename K, typename V>
LeafNode<K, V>* QTree<K, V>::findLeafNode(K* key, stack<InternalNode<K,V>*>* stackNodes, stack<int>* stackSlots) {
    Node<K, V>* node = root;
    int slot = 0;
    while (!node->isLeaf()) {
        InternalNode<K, V> *nodeInternal = (InternalNode<K, V>*) node;
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
    K* removedMax;
    bool resetMax = false;
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
                if(slot < nodeInternal->allocated && nodeInternal->keys[slot]->MinGT(key)){
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
                    if(slot >= node->allocated  || node->keys[slot]->MinGT(key)){
                        for(int i = 0; i < slot; i ++ ){
                            ((InternalNode<K, V>*) node)->checkUnderflowWithRight(i);
                        }
                        if(node->maxValue == removedMax){
                            ((InternalNode<K, V>*) node)->resetMaxValue();
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
            for(int i = 0; i < leafNode->allocated ; i ++){
//                    System.out.println("query:" + leafNode.values[i]);
                if(leafNode->values[i]->cover(attribute)){
                    if(leafNode->maxValue == leafNode->keys[i]){
                        resetMax = true;
                        removedMax = (leafNode->maxValue);
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
            if(stackNodes->empty()){
                break;
            }
            bool getAnother = false;
            while (!stackNodes->empty()){
                node = stackNodes->top();
                stackNodes->pop();
                slot = stackSlots->top();
                stackSlots->pop();

                if(slot >= node->allocated  || node->keys[slot]->MinGT(key)){
                    for(int i = 0; i < slot; i ++ ){
                        ((InternalNode<K, V>*) node)->checkUnderflowWithRight(i);
                    }
                    if(node->maxValue == removedMax){
                        ((InternalNode<K, V>*) node)->resetMaxValue();
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


template<typename K, typename V>
ostream& operator<< (ostream &out, QTree<K,V>* qTree){
    string PADDING = "                                 ";
    int elements_debug_local_recounter = 0;
    Node<K, V>* node = NULL;
    int depth = 0;
    int height = depth;
    stack<Node<K, V>*> stackNodes;
    stackNodes.push(qTree->root); // init seed, root node
    bool lastIsInternal = !qTree->root->isLeaf();
    while (!stackNodes.empty()) {
        node = stackNodes.top();
        stackNodes.pop();

        if (!node->isLeaf()) {
            InternalNode<K,V>* internalNode = (InternalNode<K, V>*) node;
            for(int i = node->allocated; i >= 0; i --){
                stackNodes.push(((InternalNode<K, V>*) node)->childs[i]);
            }

        } else {
            elements_debug_local_recounter += node->allocated;
        }
        // For Indentation
        if (lastIsInternal || !node->isLeaf()) { // Last or Curret are Internal
            depth += (lastIsInternal ? +1 : -1);
        }
        lastIsInternal = !node->isLeaf();
        int pad = PADDING.length() > (depth - 1) ? (depth - 1) : PADDING.length();
        pad = pad < 0? 0: pad;
        out << PADDING.substr(0, pad);

        if(node->isLeaf()){
            out << ((LeafNode<K,V>*)node) << endl;
        }else{
            out << ((InternalNode<K,V>*)node) << endl;

        }
    }

    out << "height=" << height << " root=" << qTree->root->getId()
        << " low=" << (qTree->low == NULL? 0: qTree->low->getId())
        << " high=" << (qTree->high == NULL? 0: qTree->high->getId())
        << " elements=" << qTree->elements << " recounter=" << elements_debug_local_recounter << endl;

    return out;

}



#endif //QTREE_QTREE_H
