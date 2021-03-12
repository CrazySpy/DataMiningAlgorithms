#ifndef HASHNODE_H_INCLUDED
#define HASHNODE_H_INCLUDED

#include <unordered_map>
#include <memory>
#include <vector>

template<typename T>
class HashNode;

template<typename T>
using HashTree = HashNode<T>;

// T is key's type.
template <typename T>
class HashNode
{
public:
    HashNode() = default;
    HashNode(const HashNode<T> &) = delete;

    void insert(const T &);
    void insert(const std::vector<T>&);
    std::shared_ptr<HashNode<T>> next(const T &);
    bool search(const T &);
    bool search(const std::vector<T> &);

private:
    std::unordered_map<T, std::shared_ptr<HashNode<T>>> _hashTable;
};

template <typename T>
void HashNode<T>::insert(const T &key)
{
    // The key has inserted into the hash tree, just return.
    if (_hashTable.count(key))
        return;

    _hashTable[key] = std::make_shared<HashNode<T>>();
}

template <typename T>
void HashNode<T>::insert(const std::vector<T> &keys)
{
    insert(keys[0]);
    std::shared_ptr<HashNode<T>> p = _hashTable[keys[0]];
    for(unsigned int i = 1; i < keys.size(); ++i)
    {
        p->insert(keys[i]);
        p = p->next(keys[i]);
    }
}

template <typename T>
std::shared_ptr<HashNode<T>> HashNode<T>::next(const T &key)
{
    if (!_hashTable.count(key))
        return nullptr;
    return _hashTable[key];
}

template <typename T>
bool HashNode<T>::search(const T &key)
{
    return _hashTable.count(key);
}

template <typename T>
bool HashNode<T>::search(const std::vector<T> &keys)
{
    if(!search(keys[0])) return false;
    std::shared_ptr<HashNode<T>> p = _hashTable[keys[0]];

    for(unsigned int i = 1; i < keys.size(); ++i)
    {
        if (!p->search(keys[i]))
            return false;
        p = p->next(keys[i]);
    }
    return true;
}

#endif