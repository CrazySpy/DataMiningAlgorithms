//
// Created by 蒋希文 on 2021/2/23.
//

#ifndef CPITREE_CPITREENODE_H
#define CPITREE_CPITREENODE_H

#include "Types.h"
#include "CPITreeNode.h"
#include <vector>

class CPITreeNode {
private:
    std::vector<std::shared_ptr<CPITreeNode>> _children;

    std::pair<FeatureType, InstanceIdType> _instanceName;

    std::shared_ptr<CPITreeNode> _indirectChild;

    std::set<std::pair<FeatureType, InstanceIdType>> _brothers;

public:
    CPITreeNode() = default;
    CPITreeNode(const std::pair<FeatureType, InstanceIdType> &);
    CPITreeNode(std::pair<FeatureType, InstanceIdType> &&instance);

    const std::pair<FeatureType, InstanceIdType>& getInstanceName();

    std::shared_ptr<CPITreeNode> addChild(const std::pair<FeatureType, InstanceIdType> &);
    bool hasChild();
    const std::vector<std::shared_ptr<CPITreeNode>>& getChildren();


    void addIndirectChild(std::shared_ptr<CPITreeNode>);
    bool hasIndirectChild();
    std::shared_ptr<CPITreeNode> getIndirectChild();

    void addBrother(std::shared_ptr<CPITreeNode>);
    void addBrother(const std::pair<FeatureType, InstanceIdType> &);
    bool hasBrother(const std::pair<FeatureType, InstanceIdType> &);
};


#endif //CPITREE_CPITREENODE_H
