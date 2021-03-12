#ifndef FPTREE_H_INCLUDED
#define FPTREE_H_INCLUDED

#include <list>
#include <vector>
#include <map>
#include <memory>

template<typename T>
class FPTree {
public:
    class FPNode {
    private:
        int _cnt; // Not the itemCount.
        T _val;
        std::shared_ptr<std::map<T, std::shared_ptr<FPNode>>> _childTable;
        std::shared_ptr<FPNode> _neighbour;
        std::shared_ptr<FPNode> _parent;

    public:
        std::shared_ptr<FPNode> insert(const T &val);

        void setNeighbour(std::shared_ptr<FPNode>);
        std::shared_ptr<FPNode> getNeighbour();

        std::shared_ptr<std::map<T, std::shared_ptr<FPNode>>> getChildTable();
        std::shared_ptr<FPNode> getChild(const T &child);
        unsigned int getChildCount();

        void addCount(int val = 1);
        unsigned int getCount();

        void setParent(std::shared_ptr<FPNode>);
        std::shared_ptr<FPNode> getParent();

        void setVal(const T &val);
        const T& getVal();

        FPNode()
            :   _cnt(0),
                _neighbour(nullptr),
                _parent(nullptr),
                _childTable(std::make_shared<std::map<T, std::shared_ptr<FPNode>>>()) {};
    };

    struct ItemInfoBlock {
        int itemCount; // The number of the item occurred in the transactions.
        std::shared_ptr<FPNode> itemTail; // Record the end of an item node, in order to link neighbour rapidly.
        std::shared_ptr<FPNode> itemHead;
    };

private:
    std::shared_ptr<std::map<T, std::shared_ptr<ItemInfoBlock>>> _itemInfo;
    std::shared_ptr<FPNode> _dummyHead; // the "null" head.
public:
    void insert(const std::vector<T> &val, int cnt = 1, std::shared_ptr<std::map<T, int>> = nullptr, int itemThreshold = 0);
    std::shared_ptr<std::map<T, std::shared_ptr<ItemInfoBlock>>> getItemInfo();
    bool empty();

    FPTree() : _dummyHead(std::make_shared<FPNode>()), _itemInfo(std::make_shared<std::map<T, std::shared_ptr<ItemInfoBlock>>>()){};

    std::shared_ptr<FPNode> getHeadNode();
};

template<typename T>
inline unsigned int FPTree<T>::FPNode::getCount() {
    return _cnt;
}

template<typename T>
inline std::shared_ptr<std::map<T, std::shared_ptr<typename FPTree<T>::ItemInfoBlock>>> FPTree<T>::getItemInfo() {
    return _itemInfo;
}

template<typename T>
inline void FPTree<T>::FPNode::setVal(const T &val) {
    _val = val;
}

template<typename T>
inline const T& FPTree<T>::FPNode::getVal() {
    return _val;
}

template<typename T>
std::shared_ptr<typename FPTree<T>::FPNode> FPTree<T>::FPNode::getParent() {
    if(!_parent) return nullptr;
    if(!_parent->_parent) return nullptr; // parent is dummyHead.
    return _parent;
}

template<typename T>
inline void FPTree<T>::FPNode::setParent(std::shared_ptr<FPNode> parent) {
    _parent = parent;
}

template<typename T>
inline void FPTree<T>::FPNode::setNeighbour(std::shared_ptr<typename FPTree<T>::FPNode> neighbour) {
    _neighbour = neighbour;
}

// itemCnt collected the numbers of occurred items in value vector.
// If the numbers of occurred items less than the itemThreshold(minimum support count), it would not be inserted.
// The itemCnt and itemThreshold are used to build conditional FP tree.
template<typename T>
void FPTree<T>::insert(
        const std::vector<T> &val,
        int cnt,
        std::shared_ptr<std::map<T, int>> itemCnt,
        int itemThreshold) {
    auto p = _dummyHead;
    for(auto &v : val) {
        if(itemCnt) {
            if((*itemCnt)[v] < itemThreshold) {
                continue;
            }
        }
        auto child = p->getChild(v);
        if(child == nullptr) {
            child = p->insert(v);

            if(!_itemInfo->count(v)) {
                (*_itemInfo)[v] = std::make_unique<typename FPTree<T>::ItemInfoBlock>();
                (*_itemInfo)[v]->itemHead = child;
            } else {
                // Link the neighbour
                (*_itemInfo)[v]->itemTail->setNeighbour(child);
            }

            (*_itemInfo)[v]->itemTail = child;
            child->setParent(p);
            child->setVal(v);
        } 

        child->addCount(cnt);
        (*_itemInfo)[v]->itemCount += cnt;
        p = child; 
    }
}

template<typename T>
std::shared_ptr<typename FPTree<T>::FPNode> FPTree<T>::FPNode::getChild(const T &child) {
    if(!_childTable->count(child)) return nullptr;
    return (*_childTable)[child];
}

template<typename T>
inline std::shared_ptr<typename FPTree<T>::FPNode> FPTree<T>::FPNode::getNeighbour() {
    return _neighbour;
}

template<typename T>
inline void FPTree<T>::FPNode::addCount(int val) {
    _cnt += val;
}

template<typename T>
inline std::shared_ptr<typename FPTree<T>::FPNode> FPTree<T>::FPNode::insert(const T &val) {
    return (*_childTable)[val] = std::make_shared<FPNode>();
}

template<typename T>
inline unsigned int FPTree<T>::FPNode::getChildCount() {
    return (*_childTable).size();
}

template<typename T>
inline bool FPTree<T>::empty() {
    return !_dummyHead->getChildCount();
}

template<typename T>
inline std::shared_ptr<typename FPTree<T>::FPNode> FPTree<T>::getHeadNode() {
    return _dummyHead;
}

template<typename T>
inline std::shared_ptr<std::map<T, std::shared_ptr<typename FPTree<T>::FPNode>>> FPTree<T>::FPNode::getChildTable() {
    return _childTable;
}

#endif