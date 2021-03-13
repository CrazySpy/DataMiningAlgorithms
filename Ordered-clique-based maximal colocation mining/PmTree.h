//
// Created by 蒋希文 on 2021/3/8.
//

#ifndef CPITREEADVANCED_PMTREE_H
#define CPITREEADVANCED_PMTREE_H

#include <map>
#include <vector>
#include "Types.h"
#include "TreeNode.h"

class PmTree {
private:
    std::shared_ptr<TreeNode<FeatureType>> _root;

    // Used to build same-feature-node link.
    // In the article, the link is denoted by dot arrow.
    std::map<FeatureType, std::shared_ptr<TreeNode<FeatureType>>> _featureTailNode;
public:
    // The constructor uses '_addSize2PrevalentColocation' function to built P2 Tree.
    PmTree(const std::vector<std::pair<FeatureType, FeatureType>> &size2PrevalentColocations);

    // The function generates candidate colocations from Pm-tree.
    std::vector<std::vector<FeatureType>> generateCandidate();

private:
    // The function builds P2-tree.
    void _addSize2PrevalentColocation(const std::pair<FeatureType, FeatureType> &colocation);

    // The function generates CPm-tree from the P2-tree built by '_addSize2PrevalentColocation' and constructor function.
    void _generateCPmTree();

    // The function traverses Pm-Tree to generate all the colocation whose size is greater than 2.
    void _generateCandidateRecursive(std::shared_ptr<TreeNode<FeatureType>> cur,
                                     std::vector<std::vector<FeatureType>> &candidates,
                                     std::vector<FeatureType> &candidate);
};


#endif //CPITREEADVANCED_PMTREE_H
