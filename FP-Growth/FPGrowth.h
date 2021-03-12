#ifndef FPGROWTH_H_INCLUDED
#define FPGROWTH_H_INCLUDED

#include <cmath>
#include <vector>
#include <memory>
#include <stack>
#include <iostream>
#include <algorithm>
#include <set>
#include "FPTree.h"

template<typename T>
using ItemSet = std::vector<T>;

template<typename T>
using ItemSetSet = ItemSet<ItemSet<T>>;

template<typename T>
using Transaction = ItemSet<T>;

template<typename T>
using TransSet = ItemSet<Transaction<T>>;

template <typename T>
class FPGrowth {
public:
    FPGrowth(TransSet<T> &transSet, double supp);
private:
    TransSet<T> &_transSet;
    double _supp;
    std::shared_ptr<FPTree<T>> _fpTree;
    std::map<ItemSet<T>, int> _itemsetCnt;

    std::shared_ptr<std::map<ItemSet<T>, int>> _freqSet;

    std::shared_ptr<std::map<T, std::shared_ptr<typename FPTree<T>::ItemInfoBlock>>> _itemInfo;

    void _generateFrequentSetRecursion(
            std::shared_ptr<FPTree<T>> conditionalFPTree,
             ItemSet<T> &suffix,
            unsigned int minCnt = -1);

    void _generateFrequentSet();

    std::shared_ptr<std::vector<std::pair<std::vector<T>, int>>>
    _generateConditionalPatternBase(
            std::shared_ptr<FPTree<T>> fpTree,
            const T &suffix,
            std::map<T, int> &itemCnt);

    // Insert transSet into FP tree.
    void _buildFPTree();
    std::shared_ptr<FPTree<T>> _buildConditionalFPTree(
            std::shared_ptr<std::vector<std::pair<std::vector<T>, int>>>,
                    std::shared_ptr<std::map<T, int>>);

public:
    std::shared_ptr<std::map<ItemSet<T>, int>> execute();
};

template<typename T>
void FPGrowth<T>::_buildFPTree() {
    for(const auto &trans : _transSet) {
        _fpTree->insert(trans);
    }

    _itemInfo = _fpTree->getItemInfo();
}

template<typename T>
std::shared_ptr<FPTree<T>> FPGrowth<T>::_buildConditionalFPTree(
        std::shared_ptr<std::vector<std::pair<std::vector<T>, int>>> conditionalPatternBase,
        std::shared_ptr<std::map<T, int>> itemCnt) {
    std::shared_ptr<FPTree<T>> conditionalTree = std::make_shared<FPTree<T>>();

    double threshold = ceil(_transSet.size() * _supp);

    for(auto &patternBase : *conditionalPatternBase) {
        int addVal = patternBase.second;
        auto &path = patternBase.first;

        conditionalTree->insert(path, addVal, itemCnt, threshold);
    }

    return conditionalTree;
}

template<typename T>
FPGrowth<T>::FPGrowth(TransSet<T> &transSet, double supp)
    : _transSet(transSet),
      _supp(supp),
      _freqSet(std::make_shared<std::map<ItemSet<T>, int>>()),
      _fpTree(std::make_shared<FPTree<T>>()) {
}

// itemCnt used to collect the number of occurred items in transactions in conditional pattern bases.
template<typename T>
std::shared_ptr<std::vector<std::pair<std::vector<T>, int>>> FPGrowth<T>::_generateConditionalPatternBase(
        std::shared_ptr<FPTree<T>> fpTree,
        const T &suffix, // The last element chosen as the suffix.
        std::map<T, int> &itemCnt) {
    auto conditionalPatternBase = std::make_shared<std::vector<std::pair<std::vector<T>, int>>>();
    auto itemInfo = fpTree->getItemInfo();

    auto p = (*itemInfo)[suffix]->itemHead;
    while(p) {
        std::vector<T> path;
        auto parent = p->getParent();
        while(parent) {
            itemCnt[parent->getVal()] += p->getCount();

            path.push_back(parent->getVal());
            parent = parent->getParent();
        }
        // not child of the "null" node
        if(path.size()) {
            reverse(path.begin(), path.end());
            conditionalPatternBase->push_back(std::make_pair(path, p->getCount()));
        }
        p = p->getNeighbour();
    }

    return conditionalPatternBase;
}

template<typename T>
void FPGrowth<T>::_generateFrequentSetRecursion(
        std::shared_ptr<FPTree<T>> conditionalFPTree,
        ItemSet<T> &suffix,
        unsigned int minCnt ) {

    std::shared_ptr<std::map<T, std::shared_ptr<typename FPTree<T>::ItemInfoBlock>>> itemInfo
        = conditionalFPTree->getItemInfo();

    std::vector<std::pair<int, T>> t;
    for(auto it = itemInfo->begin(); it != itemInfo->end(); ++it) {
        t.push_back({it->second->itemCount, it->first});
    }
    sort(t.begin(), t.end());

    for(auto it = t.begin(); it != t.end(); ++it) {
        suffix.push_back(it->second);
        _itemsetCnt[suffix] = (*itemInfo)[it->second]->itemCount;
        if((*itemInfo)[it->second]->itemCount >= ceil(_supp * _transSet.size())) {
            (*_freqSet)[suffix] = (*itemInfo)[it->second]->itemCount;
        }

        std::shared_ptr<std::map<T, int>> itemCount = std::make_shared<std::map<T, int>>(); // Collect the item count in conditional pattern base.
        auto conditionalPatternBase = _generateConditionalPatternBase(conditionalFPTree, it->second, *itemCount);
        std::shared_ptr<FPTree<T>> newConditionalFPTree = _buildConditionalFPTree(conditionalPatternBase, itemCount);

        _generateFrequentSetRecursion(newConditionalFPTree, suffix, std::min(minCnt, (unsigned int)it->first));
        suffix.pop_back();
    }
}

template<typename T>
void FPGrowth<T>::_generateFrequentSet() {
    ItemSet<T> tmp;
    _generateFrequentSetRecursion(_fpTree, tmp);
}

template <typename T>
std::shared_ptr<std::map<ItemSet<T>, int>> FPGrowth<T>::execute() {
    _buildFPTree();
    _generateFrequentSet();

    return _freqSet;
}

#endif