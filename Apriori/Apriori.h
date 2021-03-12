#ifndef APRIOR_H_INCLUDED
#define APRIOR_H_INCLUDED

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <map>
#include "HashTree.h"

template<typename T>
using ItemSet = std::vector<T>;

template<typename T>
using ItemSetSet = std::vector<ItemSet<T>>;

template<typename T>
class Apriori
{
public:
    Apriori(std::shared_ptr<ItemSetSet<T>> transSet, double supp, double conf);

    //Apriori(std::string filename, double supp, double conf, std::string fileType = "");

    struct Rule {
        ItemSet<T> antecedent;
        ItemSet<T> consequent;
        double conf;
    };

private:
    std::shared_ptr<ItemSetSet<T>> _transSet;

    double _transSize = 0;
    double _supp = 0; // support rate
    double _conf = 0; // confidence rate

    std::map<ItemSet<T>, int> _suppCount;
    std::map<int, std::shared_ptr<ItemSetSet<T>>> _freqSet;

    void _setSupp(double);
    void _setConf(double);

    // Generate frequent probable set containing k + 1 itemsets. Use F_k x F_k.
    std::shared_ptr<ItemSetSet<T>> _aprioriGen(std::shared_ptr<ItemSetSet<T>>, int k);

    // Build probable set hash tree.
    std::shared_ptr<HashTree<T>> _buildHashTree(std::shared_ptr<ItemSetSet<T>>);

    void _calculateSupportCount(
        ItemSet<T> &trans, 
        std::shared_ptr<HashNode<T>> p,
        int remainder,
        int k, 
        int lastPos
    );

    std::shared_ptr<ItemSetSet<T>> _generate1FrequentSet();
    
    // Calculate the frequent set with k element. The probSet will be insteaded by frequent set. 
    // k should greater than 0.
    void _generateKFrequentSet(
        std::shared_ptr<ItemSetSet<T>> probSet, 
        int k);

    std::shared_ptr<ItemSetSet<T>> _generateFrequentSet();

    void _generateRuleByItemset(
        ItemSet<T> &itemset,                                                // the full itemset
        ItemSet<T> &consequent,                                             // current consequent
        std::shared_ptr<ItemSet<Rule>> ans,    // Collect rules.
        int pos                                                             // current position in the full itemset.
    );

    std::shared_ptr<ItemSet<Rule>> _generateRule();

public:
    std::shared_ptr<ItemSet<Rule>> execute();
};



template<typename T>
void Apriori<T>::_setSupp(double supp)
{
    if (supp > 1.0 || supp < 0)
    {
        std::cerr << "Support rate must be a positive number not greater than 1." << std::endl;
    }
    else
    {
        _supp = supp;
    }
}

template<typename T>
void Apriori<T>::_setConf(double conf)
{
    if (conf > 1.0 || conf < 0)
    {
        std::cerr << "Confidence rate must be a positive number not greater than 1." << std::endl;
    }
    else
    {
        _conf = conf;
    }
}

template <typename T>
Apriori<T>::Apriori(std::shared_ptr<ItemSetSet<T>> transSet, double supp, double conf) : _transSize(transSet->size()), _transSet(transSet)
{
    _setSupp(supp);
    _setConf(conf);
}

template <typename T>
std::shared_ptr<ItemSetSet<T>> Apriori<T>::_aprioriGen(std::shared_ptr<ItemSetSet<T>> F, int k)
{
    std::shared_ptr<ItemSetSet<T>> probSet(new ItemSetSet<T>);
    // Choose the first F_k
    for (int i = 0; i < (int)(*F).size(); ++i)
    {
        if ((int)(*F)[i].size() != k)
            return nullptr;

        // Choose the second F_k
        for (int j = i + 1; j < (int)F->size(); ++j)
        {
            if ((int)(*F)[j].size() != k) return nullptr;

            auto &itemset1 = (*F)[i], &itemset2 = (*F)[j];
            bool canMerge = true;
            for (int p = 0; p < k - 1; ++p)
            {
                if (itemset1[p] != itemset2[p])
                {
                    canMerge = false;
                    break;
                }
            }

            if (canMerge)
            {
                // First, insert the same value.
                probSet->push_back(ItemSet<T>(itemset1.begin(), itemset1.begin() + k - 1));
                // Then, compare the last values of the two itemsets.
                // If the last of itemset1 is less than the itemset2's, push back it first.
                if (itemset1.back() < itemset2.back())
                {
                    (*probSet).back().push_back(itemset1.back());
                    (*probSet).back().push_back(itemset2.back());
                }
                else
                {
                    (*probSet).back().push_back(itemset2.back());
                    (*probSet).back().push_back(itemset1.back());
                }
            }
        }
    }

    return probSet;
}

template<typename T>
std::shared_ptr<HashTree<T>> Apriori<T>::_buildHashTree(std::shared_ptr<ItemSetSet<T>> probSet)
{
    std::shared_ptr<HashTree<T>> hashTree = std::make_shared<HashTree<T>>();
    for(auto& probItem : (*probSet))
    {
        hashTree->insert(probItem);
    }
    return hashTree;
}

template<typename T>
void Apriori<T>::_calculateSupportCount(
    ItemSet<T> &trans,                                  // Transaction set
    std::shared_ptr<HashNode<T>> p,                     // Pointer to the hash tree node.
    int remainder,                                      // The number of vocant positions in combination.
    int k,                                              // The length of combination would generate.
    int lastPos                                         // The last position of chosen item in itemset.
)
{
    if((int)trans.size() - lastPos - 1 < remainder) return; // Impossible.

    static ItemSet<T> tmp; // It's static for global-like using.
    // Temporary itemset is static, so it just initialize once.
    if(remainder == k) {
        tmp.resize(k);
    }

    if(remainder == 0) 
    {
        ++_suppCount[tmp];
        return;
    }

    for(unsigned int i = lastPos + 1; i < trans.size(); ++i)
    {
        if(!p->search(trans[i])) continue;

        tmp[k - remainder] = trans[i];
        _calculateSupportCount(trans, p->next(trans[i]), remainder - 1, k, i);
    }
}

template<typename T>
void Apriori<T>::_generateKFrequentSet(
    std::shared_ptr<ItemSetSet<T>> probSet, 
    int k)
{
    auto hashTree = _buildHashTree(probSet);
    //std::shared_ptr<std::map<ItemSet<T>, int>> suppCnt = std::make_shared<std::map<ItemSet<T>, int>>();

    // Every transaction would search in the probable hash tree.
    for(auto &trans : *_transSet)
    {
        _calculateSupportCount(trans, hashTree, k, k, -1);
    }

    auto it = probSet->begin();
    while(it != probSet->end())
    {
        if(_suppCount[*it] * 1.0 / _transSize < _supp)
        {
            it = probSet->erase(it);
        }
        else 
        {
            it++;
        }
    }
}

template<typename T>
std::shared_ptr<ItemSetSet<T>> Apriori<T>::_generate1FrequentSet()
{
    //std::map<T, int> suppCnt;
    std::shared_ptr<ItemSetSet<T>> freqSet(new ItemSetSet<T>);
    for(auto &trans : *_transSet)
    {
        for(auto &item : trans)
        {
            ItemSet<T> tmp{item};
            if((_suppCount[tmp] == 0 && _supp == 0) || ++_suppCount[tmp] == ceil(_supp * _transSize)) {
                freqSet->push_back(tmp);
            }
        }
    }

    return _freqSet[1] = freqSet;
}

template<typename T>
std::shared_ptr<ItemSetSet<T>> Apriori<T>::_generateFrequentSet()
{
    std::shared_ptr<ItemSetSet<T>> F = _generate1FrequentSet();
    for(int i = 2; ; ++i) {
        std::shared_ptr<ItemSetSet<T>> probSet = _aprioriGen(F, i - 1); // generate probable set.
        _generateKFrequentSet(probSet, i); // generateKFrequentSet function has no returns. The probSet is the K frequent set.
        if(probSet->empty()) break;
        F = _freqSet[i] = probSet;
    }

    return F;
}

template<typename T>
void Apriori<T>::_generateRuleByItemset(
    ItemSet<T> &itemset,
    ItemSet<T> &consequent,
    std::shared_ptr<ItemSet<Rule>> ans,
    int pos)
{
    // calculate subtraction which is antecedent
    ItemSet<T> antecedent;
    std::set_difference(itemset.begin(), itemset.end(), consequent.begin(), consequent.end(), std::back_inserter(antecedent));
    double conf = _suppCount[itemset] * 1.0 / _suppCount[antecedent];
    if(consequent.size() && conf < _conf) return;

    if(pos == itemset.size())
    {
        if(antecedent.empty() || consequent.empty()) return;
        ans->push_back(Rule{antecedent, consequent, conf});
        return;
    }

    _generateRuleByItemset(itemset, consequent, ans, pos + 1); // Leave itemset[i] in the antecedent.
    consequent.push_back(itemset[pos]);
    _generateRuleByItemset(itemset, consequent, ans, pos + 1); // Put itemset[i] to the consequent.
    consequent.pop_back();    // trackback
}

template<typename T>
std::shared_ptr<ItemSet<typename Apriori<T>::Rule>> Apriori<T>::_generateRule()
{
    std::shared_ptr<ItemSet<Rule>> rules = std::make_shared<ItemSet<Rule>>();
    for(int i = 2; _freqSet.count(i); ++i)
    {
        std::shared_ptr<ItemSetSet<T>> freqSet = _freqSet[i];
        for(auto &F : *freqSet)
        {
            ItemSet<T> tmp;
            _generateRuleByItemset(F, tmp, rules, 0);
        }
    }
    return rules;
}

template<typename T>
std::shared_ptr<ItemSet<typename Apriori<T>::Rule>> Apriori<T>::execute()
{
    _generateFrequentSet();

    return _generateRule();
}

#endif