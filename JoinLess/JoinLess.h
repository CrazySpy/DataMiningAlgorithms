//
// Created by 蒋希文 on 2021/2/10.
//

#ifndef JOINLESS_JOINLESS_H
#define JOINLESS_JOINLESS_H

#include "Types.h"
#include <map>
#include <set>
#include <memory>
#include <unordered_set>

class JoinLess {
private:
    double _minPI;
    double _minRuleProbability;

    std::map<FeatureType, std::map<InstanceIdType, InstanceType>> _instances;
    std::map<unsigned int, std::vector<ColocationType>> _prevalent;

    std::vector<std::pair<std::pair<FeatureType, InstanceIdType>, std::pair<FeatureType, InstanceIdType>>> _relations;
    std::map<FeatureType, std::map<InstanceIdType, std::vector<std::pair<FeatureType, InstanceIdType>>>> _starNeighborhoods;

    std::map<unsigned int, std::map<ColocationType, std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>>>> _cliqueInstances;

    std::set<std::pair<ColocationType, ColocationType>> _rules;

public:
    JoinLess(std::vector<InstanceType> &instances, double minPI, double minRuleProbability);

    std::set<RuleType> execute();

private:
    // Generate relations according to user-defined function
    // 'bool hasRelation(const InstanceType&, const InstanceType &)'.
    // The function is called by constructor function.
    void _generateRelations(std::vector<InstanceType> &instances);

    void _generateStarNeighborhoods();

    // Generate k-size candidate colocations according to (k-1)-size prevalent colocations.
    ColocationSetType _generateCandidateColocations(int k);

    // Check whether the candidate's subset is prevent.
    bool _isSubsetPrevalent(ColocationType &candidate);

    // Generate and check subset recursively.
    bool _isSubsetPrevalentRecursive(ColocationType &candidate, unsigned int pos, unsigned int remainder,
                                     std::vector<FeatureType> &tmp);

    void _selectStarInstancesRecursive(int curFeatureId, const int k, int curStarNeighborhoodId,
                                       const ColocationType &candidate,
                                       const std::vector<std::pair<FeatureType, InstanceIdType>> &starNeighborhoods,
                                       std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>> &starInstances,
                                       std::vector<std::pair<FeatureType, InstanceIdType>> &tmpInstance);

    // Generate star instances according to starNeighborhoods vector which is a star neighborhoods' set of an instance.
    std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>> _selectStarInstances(
            const std::vector<std::pair<FeatureType, InstanceIdType>> &starNeighborhoods,
            const ColocationType &candidate);

    std::map<ColocationType, std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>>>
    _filterStarInstances(const ColocationSetType &candidates, int k);

    double _calculateParticipationIndex(std::map<FeatureType, std::unordered_set<InstanceIdType>> &bitmap);

    // Prune candidates whose prevalence lower than min_pre according to their instances.
    // Current candidates' instances may not be a clique. So call the function 'coarse'.
    void _selectCoarsePrevalentColocations(std::map<ColocationType, std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>>> &);

    // Filter clique instances to _cliqueInstances.
    void _filterCliqueInstances(
            std::map<ColocationType, std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>>> &colocationInstancesMap,
            int k);

    void _selectPrevalentColocations(int k);

    void _generateRuleConsequences(const ColocationType &colocation,
        int pos,
        ColocationSetType &consequences,
        ColocationType &tmp);

    void _generateRules(unsigned int k);
};


#endif //JOINLESS_JOINLESS_H
