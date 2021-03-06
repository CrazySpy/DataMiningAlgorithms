//
// Created by 蒋希文 on 2021/2/10.
//

#include <algorithm>
#include <cmath>
#include "JoinLess.h"

JoinLess::JoinLess(std::vector<InstanceType> &instances, double minPre, double minRuleProbability, double maxDistance)
    : _minPre(minPre),
      _minRuleProbability(minRuleProbability),
      _maxDistance(maxDistance) {
    for(auto &instance : instances) {
        auto feature = std::get<Feature>(instance);
        auto id = std::get<Id>(instance);

        _instances[feature][id] = instance;
        _cliqueInstances[1][{feature}].push_back({std::make_pair(feature, id)});
    }

    // Generate P_1
    auto &prevalent = _prevalent[1];
    for(auto &instancePair : _instances) {
        prevalent.push_back({instancePair.first});
    }

    _generateRelations(instances);

    fractionScore = std::make_unique<FractionScore>(instances, _relations, _relations2);
}

bool JoinLess::_hasRelation(const InstanceType &instance1, const InstanceType &instance2) {
    auto loc1 = std::get<Location>(instance1);
    auto loc2 = std::get<Location>(instance2);

    return std::pow(loc1.first - loc2.first, 2) + std::pow(loc1.second - loc2.second, 2) <= _maxDistance * _maxDistance;
}

bool JoinLess::_hasRelation2(const InstanceType &instance1, const InstanceType &instance2) {
    auto loc1 = std::get<Location>(instance1);
    auto loc2 = std::get<Location>(instance2);

    return std::pow(loc1.first - loc2.first, 2) + std::pow(loc1.second - loc2.second, 2) <= _maxDistance * _maxDistance / 4;
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
            if(_hasRelation(instance1, instance2)) {
                // feature.id1 should less than feature2.id2.
                _relations.push_back({{feature1, id1}, {feature2, id2}});
            }

            if(_hasRelation2(instance1, instance2)) {
                _relations2.push_back({{feature1, id1}, {feature2, id2}});
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
        std::map<ColocationType, std::vector<std::vector<InstanceNameType>>> &instances,
        const std::vector<InstanceNameType> &starNeighborhoods,
        int k,
        int p,
        int remainder,
        std::vector<InstanceNameType> &tmp_instance,
        ColocationType &tmp_colocation) {

    if(!remainder) {
        instances[tmp_colocation].push_back(tmp_instance);
        return;
    }

    if(p + remainder > starNeighborhoods.size()) return;

    if(starNeighborhoods[p].first != tmp_colocation[k - remainder - 1]) {
        tmp_instance[k - remainder] = starNeighborhoods[p];
        tmp_colocation[k - remainder] = starNeighborhoods[p].first;
        _generateStarCenterSubsetInstancesRecursive(instances, starNeighborhoods, k, p + 1, remainder - 1, tmp_instance,
                                                    tmp_colocation);
    }

    _generateStarCenterSubsetInstancesRecursive(instances, starNeighborhoods, k, p + 1, remainder, tmp_instance, tmp_colocation);
}

std::map<ColocationType, std::vector<std::vector<InstanceNameType>>>
JoinLess::_generateStarCenterSubsetInstances(
        const std::vector<InstanceNameType> &starNeighborhoods,
        int k) {
    // Collect the star instances of colocations.
    std::map<ColocationType, std::vector<std::vector<InstanceNameType>>> instances;

    std::vector<InstanceNameType> tmp_instance(k);
    ColocationType tmp_colocation(k);

    tmp_instance[0] = starNeighborhoods[0];
    tmp_colocation[0] = starNeighborhoods[0].first;

    _generateStarCenterSubsetInstancesRecursive(instances, starNeighborhoods, k, 1, k - 1, tmp_instance, tmp_colocation);

    return instances;
}

std::map<ColocationType, std::vector<std::vector<InstanceNameType>>>
JoinLess::_filterStarInstances(const ColocationSetType &candidates, int k) {
    std::map<ColocationType, std::vector<std::vector<InstanceNameType>>> starInstances;

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

void JoinLess::_selectCoarsePrevalentColocations(
        std::map<ColocationType, std::vector<std::vector<InstanceNameType>>> &colocationInstanceMap) {
    if(colocationInstanceMap.empty()) return;

    auto it = colocationInstanceMap.begin();


    while(it != colocationInstanceMap.end()) {
        auto &candidate = (*it).first;
        auto &instance = (*it).second;

        // Use fraction-score instead of bitmap.
        double participationIndex = fractionScore->supportComputation(candidate, instance);

        if(participationIndex < _minPre) {
            it = colocationInstanceMap.erase(it);
        } else {
            ++it;
        }
    }

    // Clear buffer map used to complete filter 1.
    fractionScore->clearExistedInTableInstance();
}

void JoinLess::_filterCliqueInstances(
        const std::map<ColocationType, std::vector<std::vector<InstanceNameType>>> &colocationInstancesMap,
        int k) {
    for(auto it = colocationInstancesMap.begin(); it != colocationInstancesMap.end(); ++it) {
        const auto &candidate = (*it).first;
        const auto &instances = (*it).second;

        // step 9
        if(k == 2) {
            for(auto &rowInstance : instances) {
                _cliqueInstances[2][candidate].push_back(rowInstance);
            }
            continue;
        }

        ColocationType starEdgeFeature(candidate.begin() + 1, candidate.end());
        for(auto &rowInstance : instances) {
            std::vector<InstanceNameType> starEdges(rowInstance.begin() + 1, rowInstance.end());

            // If starEdges is a clique, then candidate is a clique.
            if(_cliqueInstances[k - 1].count(starEdgeFeature) &&
                std::binary_search(_cliqueInstances[k - 1][starEdgeFeature].begin(), _cliqueInstances[k - 1][starEdgeFeature].end(), starEdges)) {
                _cliqueInstances[k][candidate].push_back(rowInstance);
            }
        }
    }
}

void JoinLess::_selectPrevalentColocations(int k) {
    auto &cliqueInstances = _cliqueInstances[k];
    for(auto &candidateCliqueMap : cliqueInstances) {
        auto &candidate = candidateCliqueMap.first;
        auto &cliques = candidateCliqueMap.second;

        double participationIndex = fractionScore->supportComputation(candidate, cliques);

        if(participationIndex >= _minPre) {
            _prevalent[k].push_back(candidate);
        }
    }
}

void JoinLess::_generateRuleConsequences(
        const ColocationType &colocation,
        int pos,
        ColocationSetType &consequences,
        ColocationType &tmp) {
    if (pos == colocation.size())
    {
        if (!tmp.empty() && tmp.size() != colocation.size()) {
            consequences.push_back(tmp);
        }
        return;
    }

    _generateRuleConsequences(colocation, pos + 1, consequences, tmp);
    tmp.push_back(colocation[pos]);
    if (!tmp.empty() || std::binary_search(_prevalent[tmp.size()].begin(), _prevalent[tmp.size()].end(), tmp)) {
        _generateRuleConsequences(colocation, pos + 1, consequences, tmp);
    }
    tmp.pop_back();    // trackback
}

void JoinLess::_generateRules(unsigned int k) {
    auto &prevalentColocations = _prevalent[k];

    for (auto &colocation : prevalentColocations) {
        ColocationSetType consequences;
        ColocationType tmp;
        _generateRuleConsequences(colocation, 0, consequences, tmp);

        for (auto& consequence : consequences) {
            ColocationType antecedent;
            std::set_difference(colocation.begin(), colocation.end(), consequence.begin(), consequence.end(), std::back_inserter(antecedent));

            if (!std::binary_search(_prevalent[antecedent.size()].begin(), _prevalent[antecedent.size()].end(), antecedent)) continue;

            // Check the confidence of the candidate rule.

            auto &colocationTableInstance = _cliqueInstances[colocation.size()][colocation];
            auto &antecedentTableInstance = _cliqueInstances[antecedent.size()][antecedent];
            double conf = fractionScore->confComputation(colocation, antecedent, colocationTableInstance, antecedentTableInstance);
            if(conf >= _minRuleProbability) {
                _rules.insert({antecedent, consequence});
            }
        }

            /*
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
                _rules.insert({ antecedent, consequence });
            }
        }
             */
    }
    fractionScore->clearExistedInTableInstance();
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