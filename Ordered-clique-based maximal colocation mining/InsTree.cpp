//
// Created by 蒋希文 on 2021/3/9.
//

#include <algorithm>
#include "InsTree.h"

InsTree::InsTree(const std::vector<std::pair<InstanceNameType, InstanceNameType>> &neighbourhoods,
                 const std::vector<FeatureType> &candidate)
                 :_candidate(candidate){
    for(auto it = neighbourhoods.begin(); it != neighbourhoods.end(); ++it) {
        const InstanceNameType &instance1 = (*it).first, &instance2 = (*it).second;
        if(instance1.first == instance2.first) continue;
        if(!std::binary_search(candidate.begin(), candidate.end(), instance1.first) ||
           !std::binary_search(candidate.begin(), candidate.end(), instance2.first) ||
           instance1.first == candidate.back()) {
            continue;
        }

        if(!_subTrees.count(instance1.first)) {
            _subTrees[instance1.first] = std::make_shared<TreeNode<InstanceNameType>>();
        }

        auto headNode = _subTrees[instance1.first];

        if(!headNode->hasChild(instance1)) {
            headNode->addChild(instance1);
        }

        headNode->getChild(instance1)->addChild(instance2);
    }

    for(int i = 0; i < (int)candidate.size() - 1; ++i) {
        auto &subTreeHead = _subTrees[candidate[i]];

        auto children = subTreeHead->getChildren();
        for(auto it = children.begin(); it != children.end(); ++it) {
            auto childNode = (*it).second;
            if(childNode->getChildrenCount() < candidate.size() - i - 1) {
                // Erase the node which has less than (k - i) children.
                subTreeHead->eraseChild((*it).first);
            }
        }
    }
}

void InsTree::_buildInsTreeRecursive(std::shared_ptr<TreeNode<InstanceNameType>> root,
                                     unsigned int depth,
                                     std::vector<std::vector<InstanceNameType>> &cliques,
                                     std::vector<InstanceNameType> &tmp) {
    if(!root->hasChild()) {
        if(tmp.size() == _candidate.size()) {
            cliques.push_back(tmp);
        }
        return;
    }

    auto children = root->getChildren();
    for(auto it = children.begin(); it != children.end(); ++it) {
        auto &childName = (*it).first;
        auto &childNode = (*it).second;

        // childName.first could not smaller than candidate[depth]
        if(childName.first > _candidate[depth]) break;

        if(_subTrees.count(childName.first) && _subTrees[childName.first]->hasChild(childName)) {
            auto childChildren = _subTrees[childName.first]->getChild(childName)->getChildren();
            for(auto it2 = childChildren.begin(); it2 != childChildren.end(); ++it2) {
                auto &childChildName = (*it2).first;

                if(childNode->hasSibling(childChildName)) {
                    childNode->addChild(childChildName);
                }
            }
        }

        tmp.push_back(childName);
        _buildInsTreeRecursive(childNode, depth + 1, cliques, tmp);
        tmp.pop_back();
    }
}

std::vector<std::vector<InstanceNameType>> InsTree::_buildInsTree() {
    const FeatureType &firstFeature = _candidate.front();

    std::vector<std::vector<InstanceNameType>> cliques;
    std::vector<InstanceNameType> tmp;

    auto firstFeatureChildren = _subTrees[_candidate.front()]->getChildren();
    for(auto firstFeatureChild : firstFeatureChildren) {
        tmp.push_back(firstFeatureChild.first);
        _buildInsTreeRecursive(firstFeatureChild.second, 1, cliques, tmp);
        tmp.pop_back();
    }

    return cliques;
}

std::vector<std::vector<InstanceNameType>> InsTree::generateInstances() {
    return _buildInsTree();
}