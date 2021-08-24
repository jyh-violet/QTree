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
#include "../holder/RangeHolder.h"
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
