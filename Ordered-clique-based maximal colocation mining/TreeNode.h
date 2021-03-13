//
// Created by 蒋希文 on 2021/3/8.
//

#ifndef CPITREEADVANCED_TREENODE_H
#define CPITREEADVANCED_TREENODE_H

#include <iostream>
#include <memory>
#include <map>
#include <unordered_map>

template<typename T>
class TreeNode : public std::enable_shared_from_this<TreeNode<T>> {
private:
    T _val;

    std::map<T, std::shared_ptr<TreeNode<T>>> _children;
    std::shared_ptr<TreeNode<T>> _specialPrevious; // The pointer is used to record the special previous node.

    std::weak_ptr<TreeNode<T>> _parent;
public:
    TreeNode() = default;
    TreeNode(const T &nodeVal);

    T getVal();

    bool hasChild();
    bool hasChild(const T &childVal);
    std::shared_ptr<TreeNode<T>> addChild(const T &childVal);
    std::shared_ptr<TreeNode<T>> getChild(const T &childVal);
    std::map<T, std::shared_ptr<TreeNode<T>>> getChildren();
    unsigned int getChildrenCount();
    void eraseChild(const T &childVal);

    bool hasSibling(const T &siblingVal);


    void addSpecialPrevious(std::shared_ptr<TreeNode<T>> previous);
    bool hasSpecialPrevious();
    std::shared_ptr<TreeNode<T>> getSpecialPrevious();
};

template<typename T>
TreeNode<T>::TreeNode(const T &nodeVal)
    : _val(nodeVal){
}

template<typename T>
bool TreeNode<T>::hasChild() {
    return !_children.empty();
}

template<typename T>
bool TreeNode<T>::hasChild(const T &childVal) {
    return _children.count(childVal);
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::addChild(const T &childVal) {
    _children[childVal] = std::make_shared<TreeNode<T>>(childVal);
    _children[childVal]->_parent = std::enable_shared_from_this<TreeNode<T>>::shared_from_this();
    return _children[childVal];
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::getChild(const T &childVal) {
    if(!_children.count(childVal)) return nullptr;
    return _children[childVal];
}

template<typename T>
std::map<T, std::shared_ptr<TreeNode<T>>> TreeNode<T>::getChildren() {
    return _children;
}

template<typename T>
unsigned int TreeNode<T>::getChildrenCount() {
    return _children.size();
}

template<typename T>
void TreeNode<T>::eraseChild(const T &childVal) {
    _children.erase(childVal);
}

template<typename T>
void TreeNode<T>::addSpecialPrevious(std::shared_ptr<TreeNode<T>> previous) {
    _specialPrevious = previous;
}

template<typename T>
bool TreeNode<T>::hasSpecialPrevious() {
    return _specialPrevious != nullptr;
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::getSpecialPrevious() {
    return _specialPrevious;
}

// The function check the node's parent has the specific child node.
// If so, the node has the specific sibling node.
template<typename T>
bool TreeNode<T>::hasSibling(const T &siblingVal) {
    if(_parent.expired()) return false;
    auto parent = _parent.lock();
    return (parent->_children).count(siblingVal);
}

template<typename T>
T TreeNode<T>::getVal() {
   return _val;
}

#endif //CPITREEADVANCED_TREENODE_H
