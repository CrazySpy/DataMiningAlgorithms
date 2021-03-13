//
// Created by 蒋希文 on 2021/2/23.
//

#include "CPITree.h"
#include <list>
#include <vector>

extern bool isNeighbourhood(const InstanceType &, const InstanceType &);

CPITree::CPITree(const std::vector<InstanceType> &instances)
    : _root(std::make_shared<CPITreeNode>()) {
    _generateNeighbourhoods(instances);
    _buildTree(instances);
}

void CPITree::_generateNeighbourhoods(const std::vector<InstanceType> &instances) {
    for(unsigned int i = 0; i < instances.size(); ++i) {
        auto &instance1 = instances[i];
        for(unsigned int j = i + 1; j < instances.size(); ++j) {
            auto &instance2 = instances[j];
            if(isNeighbourhood(instance1, instance2)) {
                // Save instance2 as instance1's neighbourhood.
                _neighbourhoods[{instance1.feature, instance1.id}].push_back({instance2.feature, instance2.id});
            }
        }
    }
}

void CPITree::_buildTree(const std::vector<InstanceType> &instances) {
    // In the article, T1 and T2 should be stacks.
    // But the stack need to traverse and erase elements for many times.
    // So I decided to define T1 and T2 as lists.
    std::list<std::pair<FeatureType, InstanceIdType>> T1;
    std::list<std::shared_ptr<CPITreeNode>> T2;

    for(auto &instance : instances) {
        T1.push_back({instance.feature, instance.id});
    }

    while(!T1.empty()) {
        auto l = std::move(T1.front()); T1.pop_front();
        auto p = _root->addChild(l);
        T2.push_back(p);
        while(!T2.empty()) {
            auto p2 = T2.front(); T2.pop_front();
            auto l2 = p2->getInstanceName();
            auto &neighbourhoods = _neighbourhoods[l2];

            // Record p's children.
            std::vector<std::shared_ptr<CPITreeNode>> childNodes;

            for(auto &neighbourhood : neighbourhoods) {
                if(l2.first >= neighbourhood.first) continue;

                auto p3 = p2->addChild(neighbourhood);
                childNodes.push_back(p3);
                // Delete the neighbourhood instance from T1.
                for(auto it = T1.begin(); it != T1.end(); ) {
                    if((*it) == neighbourhood) {
                        it = T1.erase(it);
                    } else {
                        ++it;
                    }
                }

                // Delete the instance from T2.
                for(auto it = T2.begin(); it != T2.end(); ) {
                    if((*it)->getInstanceName() == neighbourhood) {
                        it = T2.erase(it);
                    } else {
                        ++it;
                    }
                }

                if(_firstNodes.count(neighbourhood)) {
                    if(_firstNodes[neighbourhood]->hasChild()) {
                        // Link the node to non-leaf-node with the same instance name.
                        p3->addIndirectChild(_firstNodes[neighbourhood]);
                    }
                } else {
                    _firstNodes[neighbourhood] = p3;
                }
            }

            // Record nodes' right brothers.
            for(unsigned int i = 0; i < childNodes.size(); ++i) {
                auto leftNode = childNodes[i];
                for(unsigned int j = i + 1; j < childNodes.size(); ++j) {
                    leftNode->addBrother(childNodes[j]);
                }
            }

            // Push all the neighbourhoods instances into the T2.
            for(auto it = childNodes.rbegin(); it != childNodes.rend(); ++it) {
                if((*it)->hasIndirectChild()) continue;
                T2.push_front(*it);
            }
        }
    }
}

void CPITree::_generateInstance(std::shared_ptr<CPITreeNode> startNode,
                                std::vector<std::shared_ptr<CPITreeNode>> &intraNodes) {
    auto &startNodeChildren = startNode->getChildren();

    // In the article, variable 'child' is socalled s
    for(auto &child : startNodeChildren) {
        if(child->hasChild()) {
            _generateInstance(child, intraNodes);
        }

        intraNodes = std::vector<std::shared_ptr<CPITreeNode>>{child};
        if(startNode != _root) {
            // startNode ∪ intraNodes forms a table instance.
            // The table instance should be size-2.
            std::vector<std::pair<FeatureType, InstanceIdType>> rowInstance;
            rowInstance.push_back(startNode->getInstanceName());
            rowInstance.push_back(child->getInstanceName());
            _cliques.push_back(std::move(rowInstance));

            if(child->hasChild() || child->hasIndirectChild()) {
                _generateNextInstance(startNode, intraNodes);
            }
        }
    }
}

// For convenience, variable 'intraNodes' stores leaf-nodes actually.
void CPITree::_generateNextInstance(std::shared_ptr<CPITreeNode> startNode,
                                    std::vector<std::shared_ptr<CPITreeNode>> &intraNodes) {
    auto lastIntraNode = intraNodes.back();

    // A node could only have either direct children or indirect child.
    if(lastIntraNode->hasChild() || lastIntraNode->hasIndirectChild()) {
        if(lastIntraNode->hasIndirectChild()) {
            // Search for clique from indirect child.
            // Or there would be duplicated nodes with the same instance name.
            lastIntraNode = lastIntraNode->getIndirectChild();
        }
        auto &lastIntraNodeChildren = lastIntraNode->getChildren();

        // 'lastIntraNodeChild' is socalled t in the article.
        for(auto &lastIntraNodeChild : lastIntraNodeChildren) {
            if(!_isIntraNodesBrother(intraNodes, lastIntraNodeChild->getInstanceName())) continue;
            intraNodes.push_back(lastIntraNodeChild);

            std::vector<std::pair<FeatureType, InstanceIdType>> rowInstance;
            rowInstance.push_back(startNode->getInstanceName());
            for(auto &linkChild : intraNodes) {
                rowInstance.push_back(linkChild->getInstanceName());
            }
            _cliques.push_back(rowInstance);

            if(lastIntraNodeChild->hasChild() || lastIntraNodeChild->hasIndirectChild()) {
                _generateNextInstance(startNode, intraNodes);
            }
            intraNodes.pop_back();
        }
    }
}

bool CPITree::_isIntraNodesBrother(const std::vector<std::shared_ptr<CPITreeNode>> &intraNodes,
                                   const std::pair<FeatureType, InstanceIdType> &instanceName) {
    for(auto &intraNode : intraNodes) {
        if(!intraNode->hasBrother(instanceName)) {
            return false;
        }
    }

    return true;
}

std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>> CPITree::execute() {
    std::vector<std::shared_ptr<CPITreeNode>> tmpIntraNodes;
    _generateInstance(_root, tmpIntraNodes);

    return _cliques;
}

