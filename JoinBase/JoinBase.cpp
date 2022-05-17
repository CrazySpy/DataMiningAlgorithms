//
// Created by 蒋希文 on 2021/1/31.
//

#include <cmath>
#include <algorithm>
#include "JoinBase.h"

extern bool isRReachable(LocationType &loc1, LocationType &loc2);

JoinBase::JoinBase(std::vector<InstanceType> &instances, double minPI, double minRuleProbability, bool fmul, double cellSize)
        : _minPI(minPI),
          _minRuleProbability(minRuleProbability),
          _fmul(fmul),
          _multiResolution(instances, minPI, cellSize) {
    for(auto it = instances.begin(); it != instances.end(); ++it) {
        auto feature = std::get<Feature>(*it);
        auto id = std::get<Id>(*it);
        auto location = std::get<Location>(*it);

        _instances[feature][id] = location;

        // Initialize 1-size prevalent colocations and its table instances.
        _prevalent[1][{feature}].push_back({id});
    }
}

bool JoinBase::_isSubsetPrevalentRecursive(
        ColocationType &candidate,
        unsigned int pos,
        unsigned int remainder,
        std::vector<FeatureType> &tmp) {
    int size = candidate.size() - 2;
    int size2 = candidate.size() - 1 - 2;

    if(!remainder) return _prevalent[candidate.size() - 1].count(tmp);
    if(pos == size) return true;
    if(remainder + pos > size) return true;

    if(_isSubsetPrevalentRecursive(candidate, pos + 1, remainder, tmp)) {
        tmp[size2 - remainder] = candidate[pos];
        return _isSubsetPrevalentRecursive(candidate, pos + 1, remainder - 1, tmp);
    }

    return false;
}

bool JoinBase::_isSubsetPrevalent(ColocationType &candidate) {
    unsigned int size = candidate.size();
    if(size <= 2) return true;

    std::vector<FeatureType> tmp(size - 1);
    tmp[size - 2] = candidate[size - 1];
    tmp[size - 3] = candidate[size - 2];

    return _isSubsetPrevalentRecursive(candidate, 0, size - 3, tmp);
}

ColocationPackage JoinBase::_generateTableInstances(ColocationSetType &candidates, int k) {
    ColocationPackage candidatePackage;
    for(auto candidate : candidates) {
        ColocationType colocation1(candidate.begin(), candidate.end() - 1);
        ColocationType colocation2(candidate.begin(), candidate.end() - 2);
        colocation2.push_back(candidate.back());

        auto &prevalent = _prevalent[k];

        auto &tableInstance1 = prevalent[colocation1];
        auto &tableInstance2 = prevalent[colocation2];

        // Merge two row instances if they are able to merge.
        for (auto it1 = tableInstance1.begin(); it1 != tableInstance1.end(); ++it1) {
            RowInstanceType &rowInstance1 = *it1;
            for (auto it2 = tableInstance2.begin(); it2 != tableInstance2.end(); ++it2) {
                RowInstanceType &rowInstance2 = *it2;

                bool canMerge = true;
                for (unsigned int idx = 0; idx < k - 1; ++idx) {
                    if (rowInstance1[idx] != rowInstance2[idx]) {
                        canMerge = false;
                        break;
                    }
                }

                if (canMerge) {
                    LocationType &location1 = _instances[colocation1.back()][rowInstance1.back()];
                    LocationType &location2 = _instances[colocation2.back()][rowInstance2.back()];
                    if (isRReachable(location1, location2)) {
                        RowInstanceType newRowInstance(rowInstance1.begin(), rowInstance1.end() - 1);
                        if (colocation1.back() < colocation2.back()) {
                            newRowInstance.push_back(rowInstance1.back());
                            newRowInstance.push_back(rowInstance2.back());
                        } else {
                            newRowInstance.push_back(rowInstance2.back());
                            newRowInstance.push_back(rowInstance1.back());
                        }

                        candidatePackage[candidate].push_back(std::move(newRowInstance));
                    }
                }
            }
        }
    }
    return candidatePackage;
}

// Apriori-gen like function.
ColocationSetType JoinBase::_generateCandidateColocations(int k) {
    ColocationSetType candidates;

    ColocationPackage &colocationPackage = _prevalent[k];
    ColocationSetType C;
    for(auto it = colocationPackage.begin(); it != colocationPackage.end(); ++it) {
        C.push_back((*it).first);
    }

    for (unsigned int i = 0; i < C.size(); ++i) {
        ColocationType &colocation1 = C[i];
        for (unsigned int j = i + 1; j < C.size(); ++j) {
            ColocationType &colocation2 = C[j];

            bool canMerge = true;
            for (unsigned int idx = 0; idx < k - 1; ++idx) {
                if (colocation1[idx] != colocation2[idx]) {
                    canMerge = false;
                    break;
                }
            }

            if (canMerge) {
                // Generate a candidate colocation by merging two colocations.
                ColocationType candidate(C[i].begin(), C[i].end() - 1);
                candidate.push_back(std::min(C[i].back(), C[j].back()));
                candidate.push_back(std::max(C[i].back(), C[j].back()));

                // Check whether all the subsets of the candidate are prevalent. If not, prune it.
                if (_isSubsetPrevalent(candidate)) {
                    candidates.push_back(candidate);
                }
            }
        }
    }

    return candidates;
}

// Explained in section 3.3
void JoinBase::_selectPrevalentColocations(const ColocationPackage &colocationPackages) {
    if(colocationPackages.empty()) return;

    for(auto it = colocationPackages.cbegin(); it != colocationPackages.cend(); ++it) {
        const ColocationType &candidate = (*it).first;
        const TableInstanceType &tableInstance = (*it).second;

        std::map<FeatureType, std::vector<bool>> bitmap;
        for(unsigned int i = 0; i < candidate.size(); ++i) {
            const FeatureType &feature = candidate[i];
            bitmap[feature] = std::vector<bool>(_instances[feature].size(), false);
        }

        for(auto &rowInstance : tableInstance)
            for(unsigned int i = 0; i < candidate.size(); ++i) {
                const FeatureType &feature = candidate[i];
                bitmap[feature][rowInstance[i] - 1] = true; // instance number starts with 1.
            }

        bool isPrevalent = true;
        for(auto it2 = bitmap.begin(); it2 != bitmap.end(); ++it2) {
            const FeatureType &feature = (*it2).first;
            const std::vector<bool> &bits = (*it2).second;

            int cnt = 0;
            for(unsigned int i = 0; i < bits.size(); ++i) {
                if(bits[i]) {
                    ++cnt;
                }
            }

            if(cnt * 1.0 / bits.size() < _minPI) {
                isPrevalent = false;
                break;
            }
        }

        if(isPrevalent) {
            unsigned int k = colocationPackages.begin()->first.size();
            _prevalent[k][candidate] = tableInstance;
        }
    }
}

void JoinBase::_generateRuleConsequents(
    const ColocationType &colocation,
    int pos,
    ColocationSetType &consequents,
    ColocationType &tmp) {
    if (pos == colocation.size())
    {
        if (!tmp.empty() && tmp.size() != colocation.size()) {
            consequents.push_back(tmp);
        }
        return;
    }

    _generateRuleConsequents(colocation, pos + 1, consequents, tmp);
    tmp.push_back(colocation[pos]);
    if (!tmp.empty() || _prevalent[tmp.size()].count(tmp)) {
        _generateRuleConsequents(colocation, pos + 1, consequents, tmp);
    }
    tmp.pop_back();    // trackback
}

void JoinBase::_generateRules(unsigned int k) {
    auto &prevalent = _prevalent[k + 1];

    for (auto &colocationPackage : prevalent) {
        auto &colocation = colocationPackage.first;

        ColocationSetType consequents;
        ColocationType tmp;
        _generateRuleConsequents(colocation, 0, consequents, tmp);

        for (auto &consequent : consequents) {
            ColocationType antecedent;
            std::set_difference(colocation.begin(), colocation.end(), consequent.begin(), consequent.end(), std::back_inserter(antecedent));
            
            if (!_prevalent[antecedent.size()].count(antecedent)) continue;

            // Check the confidence of the candidate rule.

            // The variable stores the vector ids, which indicate the features allocated in the antecedent.
            std::vector<unsigned int> idsInColocation;
            int p = 0;
            for (unsigned int i = 0; i < colocation.size(); ++i) {
                if (colocation[i] == antecedent[p]) {
                    idsInColocation.push_back(i);
                    p++;
                }
                if (p == antecedent.size()) break;
            }
            
            // The variable stores the different instances which have the same features with antecedent.
            std::set<std::vector<InstanceIdType>> differentInstances;
            auto &colocationTableInstance = prevalent[colocation];

            for (auto rowInstance : colocationTableInstance) {
                std::vector<InstanceIdType> antecedentRowInstancesInColocation;
                for (unsigned int i = 0; i < idsInColocation.size(); ++i) {
                    antecedentRowInstancesInColocation.push_back(rowInstance[idsInColocation[i]]);
                }
                differentInstances.insert(antecedentRowInstancesInColocation);
            }

            unsigned int antecedentTableInstanceSize = _prevalent[antecedent.size()][antecedent].size();
            if (differentInstances.size() * 1.0 / antecedentTableInstanceSize >= _minRuleProbability) {
                _rules.insert({antecedent, consequent});
            }
        }
    }
}

std::set<RuleType> JoinBase::execute() {
    unsigned int k = 1;
    while(_prevalent.count(k) && !_prevalent[k].empty()) {
        ColocationSetType candidates = _generateCandidateColocations(k);
        if(_fmul) {
            _multiResolution.multiResolutionPruning(candidates, k);
        }
        ColocationPackage candidatePackage = _generateTableInstances(candidates, k);
        _selectPrevalentColocations(candidatePackage);

        _generateRules(k);
        k++;
    }

    return _rules;
}

