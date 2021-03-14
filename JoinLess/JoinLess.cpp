//
// Created by 蒋希文 on 2021/2/10.
//

#include "JoinLess.h"
#include <algorithm>

extern bool hasRelation(const InstanceType &, const InstanceType &);

JoinLess::JoinLess(std::vector<InstanceType> &instances, double minPre, double minRuleProbability)
    : _minPre(minPre),
      _minRuleProbability(minRuleProbability) {
    for(auto &instance : instances) {
        auto feature = std::get<Feature>(instance);
        auto id = std::get<Id>(instance);

        _instances[feature][id] = instance;
        _cliqueInstances[1][{feature}].insert({std::make_pair(feature, id)});
    }

    // Generate P_1
    auto &prevalence = _prevalent[1];
    for(auto &instancePair : _instances) {
        prevalence.push_back({instancePair.first});
    }

    _generateRelations(instances);
}

void JoinLess::_generateRelations(std::vector<InstanceType> &instances) {
    for(unsigned int i = 0; i < instances.size(); ++i) {
        InstanceType &instance1 = instances[i];
        FeatureType feature1 = std::get<Feature>(instance1);
        InstanceIdType id1 = std::get<Id>(instance1);
        for(unsigned int j = i + 1; j < instances.size(); ++j) {
            InstanceType &instance2 = instances[j];
            FeatureType feature2 = std::get<Feature>(instance2);
            InstanceIdType id2 = std::get<Id>(instance2);
            if(hasRelation(instance1, instance2)) {
                // feature.id1 should less than feature2.id2.
                _relations.push_back({{feature1, id1}, {feature2, id2}});
            }
        }
    }
}

void JoinLess::_generateStarNeighborhoods() {
    for(auto &relation : _relations) {
        auto &starCenter = relation.first;
        auto &starEdge = relation.second;

        // The star neighborhoods include the star center self.
        if(!_starNeighborhoods.count(starCenter.first) || !_starNeighborhoods[starCenter.first].count(starCenter.second)) {
            _starNeighborhoods[starCenter.first][starCenter.second].push_back(starCenter);
        }

        _starNeighborhoods[starCenter.first][starCenter.second].push_back(starEdge);
    }
}

bool JoinLess::_isSubsetPrevalentRecursive(
        ColocationType &candidate,
        unsigned int pos,
        unsigned int remainder,
        std::vector<FeatureType> &tmp) {
    int size = candidate.size() - 2;
    int size2 = candidate.size() - 1 - 2;

    if(!remainder) {
        auto &prevalent = _prevalent[candidate.size() - 1];
        return std::binary_search(prevalent.begin(), prevalent.end(), tmp);
    }
    if(pos == size) return true;
    if(remainder + pos > size) return true;

    if(_isSubsetPrevalentRecursive(candidate, pos + 1, remainder, tmp)) {
        tmp[size2 - remainder] = candidate[pos];
        return _isSubsetPrevalentRecursive(candidate, pos + 1, remainder - 1, tmp);
    }

    return false;
}

bool JoinLess::_isSubsetPrevalent(ColocationType &candidate) {
    unsigned int size = candidate.size();
    if(size <= 2) return true;

    std::vector<FeatureType> tmp(size - 1);
    tmp[size - 2] = candidate[size - 1];
    tmp[size - 3] = candidate[size - 2];

    return _isSubsetPrevalentRecursive(candidate, 0, size - 3, tmp);
}

ColocationSetType JoinLess::_generateCandidateColocations(int k) {
    auto &prevalent = _prevalent[k - 1];

    ColocationSetType candidates;
    for (auto it1 = prevalent.begin(); it1 != prevalent.end(); ++it1) {
        auto &colocation1 = (*it1);

        for(auto it2 = it1 + 1; it2 !=prevalent.end(); ++it2) {
            auto &colocation2 = (*it2);

            bool canMerge = true;
            for (unsigned int idx = 0; idx < k - 2; ++idx) {
                if (colocation1[idx] != colocation2[idx]) {
                    canMerge = false;
                    break;
                }
            }

            if (canMerge) {
                // Generate a candidate colocation by merging two colocations.
                ColocationType candidate(colocation1.begin(), colocation1.end() - 1);
                candidate.push_back(std::min(colocation1.back(), colocation2.back()));
                candidate.push_back(std::max(colocation1.back(), colocation2.back()));

                // Check whether all the subsets of the candidate are prevalent.
                // If not, prune it.
                if (_isSubsetPrevalent(candidate)) {
                    candidates.push_back(candidate);
                }
            }
        }
    }

    return candidates;
}

void JoinLess::_generateStarCenterSubsetInstancesRecursive(
        std::map<ColocationType, std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>>> &instances,
        const std::vector<std::pair<FeatureType, InstanceIdType>> &starNeighborhoods,
        int k,
        int p,
        int remainder,
        std::vector<std::pair<FeatureType, InstanceIdType>> &tmp_instance,
        ColocationType &tmp_colocation) {

    if(!remainder) {
        instances[tmp_colocation].push_back(tmp_instance);
        return;
    }

    if(p + remainder > starNeighborhoods.size()) return;

    _generateStarCenterSubsetInstancesRecursive(instances, starNeighborhoods, k, p + 1, remainder, tmp_instance, tmp_colocation);

    tmp_instance[k - remainder] = starNeighborhoods[p];
    tmp_colocation[k - remainder] = starNeighborhoods[p].first;
    _generateStarCenterSubsetInstancesRecursive(instances, starNeighborhoods, k, p + 1, remainder - 1, tmp_instance, tmp_colocation);
}

std::map<ColocationType, std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>>>
        JoinLess::_generateStarCenterSubsetInstances(
                const std::vector<std::pair<FeatureType, InstanceIdType>> &starNeighborhoods,
                int k) {
    // Collect the star instances of colocations.
    std::map<ColocationType, std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>>> instances;

    std::vector<std::pair<FeatureType, InstanceIdType>> tmp_instance(k);
    ColocationType tmp_colocation(k);

    tmp_instance[0] = starNeighborhoods[0];
    tmp_colocation[0] = starNeighborhoods[0].first;

    _generateStarCenterSubsetInstancesRecursive(instances, starNeighborhoods, k, 1, k - 1, tmp_instance, tmp_colocation);

    return instances;
}

std::map<ColocationType, std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>>>
        JoinLess::_filterStarInstances(const ColocationSetType &candidates, int k) {
    std::map<ColocationType, std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>>> starInstances;

    // Collect all features which will be searched as star centers in star neighborhoods.
    std::set<FeatureType> firstFeatures;
    for(auto &candidate : candidates) {
        firstFeatures.insert(candidate[0]);
    }

    for(auto &starCenterFeature : firstFeatures) {
        auto &idStarNeighborhood = _starNeighborhoods[starCenterFeature];
        for(auto &idStarNeighborhoodPair : idStarNeighborhood) {
            auto &starCenterId = idStarNeighborhoodPair.first;
            auto &starNeighborhoods = idStarNeighborhoodPair.second;

            if(starNeighborhoods.size() < k) continue;

            // Elements in colocationInstancesMap of different iteration rounds must be distinct.
            auto colocationInstancesMap = _generateStarCenterSubsetInstances(starNeighborhoods, k);
            // Merge the old instances with the new instances generated above.
            for(auto &candidate : candidates) {
                if(candidate[0] > starCenterFeature) break; // Because candidates are sorted by feature.
                if(candidate[0] < starCenterFeature) continue;
                if(!colocationInstancesMap.count(candidate)) continue;

                auto &newInstances = colocationInstancesMap[candidate];
                auto &instances = starInstances[candidate];
                for(auto &newInstance : newInstances) {
                    instances.push_back(std::move(newInstance));
                }
            }
        }
    }

    return starInstances;
}

double JoinLess::_calculateParticipationIndex(std::map<FeatureType, std::vector<bool>> &bitmap) {
    double participationIndex = 1;
    for(auto it = bitmap.begin(); it != bitmap.end(); ++it) {
        int cnt = 0;
        auto &featureBits = (*it).second;
        for(auto bit : featureBits) {
            if(bit) {
                ++cnt;
            }
        }

        participationIndex = std::min(cnt * 1.0 / featureBits.size(), participationIndex);
    }
    return participationIndex;
}

void JoinLess::_selectCoarsePrevalentColocations(
        std::map<ColocationType, std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>>> &colocationInstancesMap) {
    if(colocationInstancesMap.empty()) return;

    auto it = colocationInstancesMap.begin();


    while(it != colocationInstancesMap.end()) {
        auto &candidate = (*it).first;
        auto &instances = (*it).second;

        std::map<FeatureType , std::vector<bool>> bitmap;

        // Initialize bitmap
        for(auto &feature : (*it).first) {
            bitmap[feature] = std::vector<bool>(_instances[feature].size(), false);
        }

        for(auto &rowInstance : instances) {
            for(auto &instance : rowInstance) {
                bitmap[instance.first][instance.second - 1] = true;
            }
        }

        double participationIndex = _calculateParticipationIndex(bitmap);
        if(participationIndex < _minPre) {
            it = colocationInstancesMap.erase(it);
        } else {
            ++it;
        }
    }
}

void JoinLess::_filterCliqueInstances(
        const std::map<ColocationType, std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>>> &colocationInstancesMap,
        int k) {
    for(auto it = colocationInstancesMap.begin(); it != colocationInstancesMap.end(); ++it) {
        const auto &candidate = (*it).first;
        const auto &instances = (*it).second;

        // step 9
        if(k == 2) {
            for(auto &rowInstance : instances) {
                _cliqueInstances[2][candidate].insert(rowInstance);
            }
            continue;
        }

        ColocationType starEdgeFeature(candidate.begin() + 1, candidate.end());
        for(auto &rowInstance : instances) {
            std::vector<std::pair<FeatureType, InstanceIdType>> starEdges(rowInstance.begin() + 1, rowInstance.end());

            // If starEdges is a clique, then candidate is a clique.
            if(_cliqueInstances[k - 1].count(starEdgeFeature) && _cliqueInstances[k - 1][starEdgeFeature].count(starEdges)) {
                _cliqueInstances[k][candidate].insert(rowInstance);
            }
        }
    }
}

void JoinLess::_selectPrevalentColocations(int k) {
    auto &cliqueInstances = _cliqueInstances[k];
    for(auto &candidateCliqueMap : cliqueInstances) {
        auto &candidate = candidateCliqueMap.first;
        auto &cliques = candidateCliqueMap.second;

        std::map<FeatureType , std::vector<bool>> bitmap;
        for(auto &feature : candidate) {
            bitmap[feature] = std::vector<bool>(_instances[feature].size(), false);
        }

        for(auto &rowInstance : cliques) {
            for(auto &instance : rowInstance) {
                auto &feature = instance.first;
                auto &id = instance.second;

                bitmap[feature][id - 1] = true;
            }
        }

        double participationIndex = _calculateParticipationIndex(bitmap);
        if(participationIndex >= _minPre) {
            _prevalent[k].push_back(candidate);
        }
    }
}

void JoinLess::_generateRuleConsequents(
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
    if (!tmp.empty() || std::binary_search(_prevalent[tmp.size()].begin(), _prevalent[tmp.size()].end(), tmp)) {
        _generateRuleConsequents(colocation, pos + 1, consequents, tmp);
    }
    tmp.pop_back();    // trackback
}

void JoinLess::_generateRules(unsigned int k) {
    auto &prevalentColocations = _prevalent[k];

    for (auto &colocation : prevalentColocations) {
        ColocationSetType consequents;
        ColocationType tmp;
        _generateRuleConsequents(colocation, 0, consequents, tmp);

        for (auto& consequent : consequents) {
            ColocationType antecedent;
            std::set_difference(colocation.begin(), colocation.end(), consequent.begin(), consequent.end(), std::back_inserter(antecedent));

            if (!std::binary_search(_prevalent[antecedent.size()].begin(), _prevalent[antecedent.size()].end(), antecedent)) continue;

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

            auto& tableInstance = _cliqueInstances[k][colocation];
            for (auto rowInstance : tableInstance) {
                std::vector<InstanceIdType> antecedentRowInstancesInColocation;
                for (unsigned int i = 0; i < idsInColocation.size(); ++i) {
                    antecedentRowInstancesInColocation.push_back(rowInstance[idsInColocation[i]].second);
                }
                differentInstances.insert(antecedentRowInstancesInColocation);
            }

            unsigned int antecedentTableInstanceSize = _cliqueInstances[antecedent.size()][antecedent].size();
            if (differentInstances.size() * 1.0 / antecedentTableInstanceSize >= _minRuleProbability) {
                _rules.insert({ antecedent, consequent });
            }
        }
    }
}

std::set<RuleType> JoinLess::execute() {
    _generateStarNeighborhoods();
    int k = 2;
    while(!_prevalent[k - 1].empty()) {
        ColocationSetType candidates = _generateCandidateColocations(k);
        auto SI = _filterStarInstances(candidates, k);
        if(k == 2) {
            // Step 9 is done in _filterCliqueInstances function.
            // The function do special when k = 2.
            _filterCliqueInstances(SI, 2);
        } else {
            _selectCoarsePrevalentColocations(SI); // SI would be changed. And the key of SI is the C_k in step 10.
            _filterCliqueInstances(SI, k); // clique instances CI is the private member '_cliqueInstances'.
        }

        _selectPrevalentColocations(k);
        _generateRules(k);
        ++k;
    }

    return _rules;
}