//
// Created by 蒋希文 on 2021/3/9.
//

#ifndef CPITREEADVANCED_INSTREE_H
#define CPITREEADVANCED_INSTREE_H

#include <map>
#include "TreeNode.h"
#include "Types.h"


// In the article, the author claimed to generate InsTree by copying nodes of NeibTree to InsTree.
// Obviously, the copying is deep-copy.
// But using deep copy to build the InsTree might not worth. Because various candidate colocations make InsTree only used for one time.
// After having used one InsTree to generate one candidate's table instances,
// we need to start another deep-copy to build another InsTree to fit the next candidate colocation.
// Also, It is not worth wasting much of time to implement algorithms to modify InsTree to fit another candidate colocation,
// even find the algorithms cost so much of time or memory after implementing them.
// So I chose to directly build one InsTree for one candidate colocation.
class InsTree {
private:
    // In order to reuse code, the NeibTree doesn't store the root described in the article.
    // With the same reason,  the member variable only store instance part,
    // and the head nodes' features denote by the variable's key.
    std::map<FeatureType, std::shared_ptr<TreeNode<InstanceNameType>>> _subTrees;

    std::vector<FeatureType> _candidate;

public:
    InsTree(const std::vector<std::pair<InstanceNameType, InstanceNameType>> &neighbourhoods,
            const std::vector<FeatureType> &candidate);

    std::vector<std::vector<InstanceNameType>> generateInstances();

private:
    void
    _buildInsTreeRecursive(std::shared_ptr<TreeNode<InstanceNameType>> root,
                           unsigned int depth, std::vector<std::vector<InstanceNameType>> &cliques,
                           std::vector<InstanceNameType> &tmp);

    std::vector<std::vector<InstanceNameType>> _buildInsTree();
};


#endif //CPITREEADVANCED_INSTREE_H
