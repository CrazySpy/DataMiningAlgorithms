//
// Created by 蒋希文 on 2021/2/23.
//

#ifndef CPITREE_CPITREE_H
#define CPITREE_CPITREE_H

#include "Types.h"
#include "CPITreeNode.h"
#include <vector>

class CPITree {
private:
    // In the article, it is called NT.
    std::map<std::pair<FeatureType, InstanceIdType>, std::vector<std::pair<FeatureType, InstanceIdType>>> _neighbourhoods;

    std::shared_ptr<CPITreeNode> _root;

    // In the article, linking nodes to the first node with same instance name is needed,
    // so I need a map to save such first nodes.
    std::map<std::pair<FeatureType, InstanceIdType>, std::shared_ptr<CPITreeNode>> _firstNodes;

    std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>> _cliques;
public:
    CPITree(const std::vector<InstanceType> &instances);
    std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>> execute();

private:
    void _generateNeighbourhoods(const std::vector<InstanceType> &instances);

    void _buildTree(const std::vector<InstanceType> &instances);

    // The variable 'startNode' is socalled 'α',
    // and the variable 'intraNodes' is socalled 'β'.
    // Actually, variable 'intraNodes' is not completely 'intra-node' defined in the article,
    // as a fact that the variable contains leaf-node while the definition doesn't agree with it.
    // However, names for the variables are needed, calling them 'alpha' or 'beta' is so ambiguous after all.
    void _generateInstance(std::shared_ptr<CPITreeNode> startNode,
                           std::vector<std::shared_ptr<CPITreeNode>> &intraNodes);

    // Actually, variable 'intraNodes' is not completely 'intra-node' defined in the article,
    // as a fact that the variable contains leaf-node while the definition doesn't agree with it.
    void _generateNextInstance(std::shared_ptr<CPITreeNode> startNode,
                               std::vector<std::shared_ptr<CPITreeNode>> &intraNodes);

    // The function is what socalled 'ok' in the article.
    // The function checks whether the specific instance is the brother of all the nodes of intra-nodes.
    bool _isIntraNodesBrother(const std::vector<std::shared_ptr<CPITreeNode>> & intraNodes,
                              const std::pair<FeatureType, InstanceIdType> &instanceName);
};


#endif //CPITREE_CPITREE_H
