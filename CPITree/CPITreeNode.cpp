//
// Created by 蒋希文 on 2021/2/23.
//

#include "CPITreeNode.h"

CPITreeNode::CPITreeNode(const std::pair<FeatureType, InstanceIdType> &instance)
    : _instanceName(instance) {
}

CPITreeNode::CPITreeNode(std::pair<FeatureType, InstanceIdType> &&instance)
        : _instanceName(std::move(instance)) {
}

std::shared_ptr<CPITreeNode> CPITreeNode::addChild(const std::pair<FeatureType, InstanceIdType> &child) {
    _children.push_back(std::make_shared<CPITreeNode>(child));
    return _children.back();
}

const std::pair<FeatureType, InstanceIdType>& CPITreeNode::getInstanceName() {
    return _instanceName;
}

bool CPITreeNode::hasChild() {
    return !_children.empty();
}

void CPITreeNode::addIndirectChild(std::shared_ptr<CPITreeNode> indirectChild) {
    _indirectChild = indirectChild;
}

bool CPITreeNode::hasIndirectChild() {
    return _indirectChild != nullptr;
}

const std::vector<std::shared_ptr<CPITreeNode>> &CPITreeNode::getChildren() {
    return _children;
}

std::shared_ptr<CPITreeNode> CPITreeNode::getIndirectChild() {
    return _indirectChild;
}

void CPITreeNode::addBrother(std::shared_ptr<CPITreeNode> brotherNode) {
    _brothers.insert(brotherNode->getInstanceName());
}

void CPITreeNode::addBrother(const std::pair<FeatureType, InstanceIdType> &instanceName) {
    _brothers.insert(instanceName);
}

bool CPITreeNode::hasBrother(const std::pair<FeatureType, InstanceIdType> &instanceName) {
    return _brothers.count(instanceName);
}


