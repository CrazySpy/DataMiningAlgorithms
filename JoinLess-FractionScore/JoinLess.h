//
// Created by 蒋希文 on 2021/2/10.
//

#ifndef JOINLESS_JOINLESS_H
#define JOINLESS_JOINLESS_H

#include "Types.h"
#include <map>
#include <set>
#include <memory>
#include "FractionScore.h"

class JoinLess {
private:
    double _minPre;
    double _minRuleProbability;
    double _maxDistance;

    std::map<FeatureType, std::map<InstanceIdType, InstanceType>> _instances;
    std::map<unsigned int, std::vector<ColocationType>> _prevalent;

    std::vector<std::pair<InstanceNameType, InstanceNameType>> _relations;

    // The variable used to store pairwise instances which distance is lower than half of the maxDistance.
    // Fraction-score needs the variable.
    std::vector<std::pair<InstanceNameType, InstanceNameType>> _relations2;

    std::map<FeatureType, std::map<InstanceIdType, std::vector<InstanceNameType>>> _starNeighborhoods;

    std::map<unsigned int, std::map<ColocationType, std::vector<std::vector<InstanceNameType>>>> _cliqueInstances;

    std::set<std::pair<ColocationType, ColocationType>> _rules;

    std::unique_ptr<FractionScore> fractionScore;

public:
    JoinLess(std::vector<InstanceType> &instances, double minPre, double minRuleProbability, double maxDistance);

    std::set<RuleType> execute();

private:
    bool _hasRelation(const InstanceType &instance1, const InstanceType &instance2);

    // Fraction-score need calculate disk(o, d/2) for filtering.
    // So the function generates such pairwise relations.
    bool _hasRelation2(const InstanceType &instance1, const InstanceType &instance2);

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

    void _generateStarCenterSubsetInstancesRecursive(
            std::map<ColocationType, std::vector<std::vector<InstanceNameType>>> &instances,
            const std::vector<InstanceNameType> &starNeighborhoods, int k, int p, int remainder,
            std::vector<InstanceNameType> &tmp_instance, ColocationType &tmp_colocation);

    // Generate star instances according to starNeighborhoods vector which is a star neighborhoods' set of an instance.
    std::map<ColocationType, std::vector<std::vector<InstanceNameType>>>
    _generateStarCenterSubsetInstances(const std::vector<InstanceNameType> &starNeighborhoods,
                                       int k);

    std::map<ColocationType, std::vector<std::vector<InstanceNameType>>>
    _filterStarInstances(const ColocationSetType &candidates, int k);

    // Prune candidates whose prevalence lower than min_pre according to their instances.
    // Current candidates' instances may not be a clique. So call the function 'coarse'.
    void _selectCoarsePrevalentColocations(std::map<ColocationType, std::vector<std::vector<InstanceNameType>>> &);

    // Filter clique instances to _cliqueInstances.
    void _filterCliqueInstances(
            const std::map<ColocationType, std::vector<std::vector<InstanceNameType>>> &colocationTableInstanceMap,
            int k);

    void _selectPrevalentColocations(int k);

    void _generateRuleConsequences(const ColocationType &colocation,
                                  int pos,
                                  ColocationSetType &consequences,
                                  ColocationType &tmp);

    void _generateRules(unsigned int k);
};


#endif //JOINLESS_JOINLESS_H
