//
// Created by 蒋希文 on 2021/3/8.
//

#include <stack>
#include <list>
#include "PmTree.h"

PmTree::PmTree(const std::vector<std::pair<FeatureType, FeatureType>> &size2PrevalentColocations)
: _root(std::make_shared<TreeNode<FeatureType>>()){
    // Build P2-tree.
    for(auto &prevalentColocation : size2PrevalentColocations) {
        _addSize2PrevalentColocation(prevalentColocation);
    }

    _generateCPmTree();
}

void PmTree::_addSize2PrevalentColocation(const std::pair<FeatureType, FeatureType> &colocation) {
    FeatureType feature1 = colocation.first;
    FeatureType feature2 = colocation.second;

    std::shared_ptr<TreeNode<FeatureType>> p;
    if(!_root->hasChild(feature1)) {
        p = _root->addChild(feature1);
        if(_featureTailNode.count(feature1)) {
            p->addSpecialPrevious(_featureTailNode[feature1]);
        }
        _featureTailNode[feature1] = p;
    } else {
        p = _root->getChild(feature1);
    }

    if(!p->hasChild(feature2)) {
        p = p->addChild(feature2);
        if(_featureTailNode.count(feature2)) {
            p->addSpecialPrevious(_featureTailNode[feature2]);
        }
        _featureTailNode[feature2] = p;
    }
}

void PmTree::_generateCPmTree() {
    // The variable stores nodes which are added later in the following algorithm.
    // In other words, the variable stores all the nodes which are not added in the '_addSize2PrevalentColocation' function.
    std::map<FeatureType, std::vector<std::shared_ptr<TreeNode<FeatureType>>>> laterAdded;

    // The variable is so-called 'Hc'
    std::deque<std::shared_ptr<TreeNode<FeatureType>>> heads;

    auto rootChildren = _root->getChildren();
    for(auto it = rootChildren.rbegin(); it != rootChildren.rend(); ++it) {
        auto firstHeadNode = (*it).second;
        heads.push_front(firstHeadNode);

        // Check current head node.
        auto firstHeadChildren = firstHeadNode->getChildren();
        if (firstHeadNode->hasSpecialPrevious()) {
            std::shared_ptr<TreeNode<FeatureType>> previous = firstHeadNode;
            while (previous = previous->getSpecialPrevious()) {
                for (auto &headChild : firstHeadChildren) {
                    auto headChildFeature = headChild.first;
                    auto headChildPointer = headChild.second;
                    if (previous->hasSibling(headChildFeature)) {
                        auto newNode = previous->addChild(headChildFeature);
                        laterAdded[headChildFeature].push_back(newNode);
                    }
                }
            }
        }

        // Check other head nodes in the right of the first head node.

        // The variable 'headNode' is so-called ψc;
        for(auto headNode = heads.begin() + 1; headNode != heads.end(); ++headNode) {
            auto headFeature = (*headNode)->getVal();
            auto headChildren = (*headNode)->getChildren();

            // Fetch all the later added nodes which have the same feature as current head node.
            auto &laterAddedNodes = laterAdded[headFeature];
            for(auto it2 = laterAddedNodes.begin(); it2 != laterAddedNodes.end(); ++it2) {
                auto laterAddedNode = (*it2);
                for(auto it3 = headChildren.begin(); it3 != headChildren.end(); ++it3) {
                    if(laterAddedNode->hasSibling((*it3).first)) {
                        auto newNode = laterAddedNode->addChild((*it3).first);
                        laterAdded[(*it3).first].push_back(newNode);
                    }
                }
            }
        }
    }
}

void PmTree::_generateCandidateRecursive(
        std::shared_ptr<TreeNode<FeatureType>> cur,
        std::vector<std::vector<FeatureType>> &candidates,
        std::vector<FeatureType> &candidate) {
    if(!cur->hasChild()) {
        if(candidate.size() > 2) {
            candidates.push_back(candidate);
        }

        return;
    }

    auto children = cur->getChildren();

    for(auto it = children.begin(); it != children.end(); ++it) {
        candidate.push_back((*it).first);
        _generateCandidateRecursive((*it).second, candidates, candidate);
        candidate.pop_back();
    }
}

// The function traverses Pm-tree recursively in way of deep-first strategy.
// First, call the '_generateCandidateRecursive' to generate colocations whose size are greater than 2.
// And then, use two pointers to eliminate covered colocations.
// For example, we get colocation {A B C D} and {A C D}, the {A C D} should be eliminated,
// because the set is covered by {A B C D}.
// Actually, the complexity is O(maxlen*(N^2)), where N is the number of colocations whose size are greater than 2, and maxlen is the maximum colocation size.
std::vector<std::vector<FeatureType>> PmTree::generateCandidate() {
    std::vector<std::vector<FeatureType>> candidates;

    // Generate colocations whose size are greater than two.
    std::vector<FeatureType> tmp;
    _generateCandidateRecursive(_root, candidates, tmp);

    // Eliminate covered colocations.
    // Using two-pointer method.
    for(auto it1 = candidates.begin(); it1 != candidates.end(); ++it1) {
        auto &colocation1 = *it1;
        for(auto it2 = candidates.begin(); it2 != candidates.end(); ) {
            auto &colocation2 = *it2;
            if(colocation1.size() < colocation2.size() || it1 == it2) {
                ++it2;
                continue;
            }

            auto pointer1 = colocation1.begin(), pointer2 = colocation2.begin();
            while(pointer1 != colocation1.end() && pointer2 != colocation2.end()) {
                if(*pointer1 == *pointer2) {
                    ++pointer2;
                }
                ++pointer1;
            }

            if(pointer2 == colocation2.end()) {
                // Colocation2 is covered by colocation1.
                it2 = candidates.erase(it2);
            } else {
                ++it2;
            }
        }
    }

    return candidates;
}