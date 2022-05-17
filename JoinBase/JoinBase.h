//
// Created by 蒋希文 on 2021/1/31.
//

#ifndef JOINBASE_JOINBASE_H
#define JOINBASE_JOINBASE_H

#include <vector>
#include <map>
#include <set>
#include "Types.h"
#include "MultiResolution.h"

class JoinBase {
private:
    double _minPI;
    double _minRuleProbability;

    bool _fmul;
    MultiResolution _multiResolution;

    std::map<FeatureType, std::map<InstanceIdType, LocationType>> _instances;

    std::map<unsigned int, ColocationPackage> _prevalent;

    std::set<RuleType> _rules;

public:
    JoinBase(std::vector<InstanceType> &instances,
             double minPI,
             double minRuleProbability,
             bool fmul = true,
             double cellSize= 1);

private:
    // generate candidate colocation from previous colocation set C_k.
    ColocationSetType _generateCandidateColocations(int k);

    ColocationPackage _generateTableInstances(ColocationSetType &candidates, int k);


    bool _isSubsetPrevalentRecursive(
            ColocationType &candidate,
            unsigned int pos,
            unsigned int remainder,
            std::vector<FeatureType> &tmp);
    bool _isSubsetPrevalent(ColocationType &candidate);

    void _selectPrevalentColocations(const ColocationPackage &colocationPackages);

    void _generateRuleConsequents(const ColocationType& colocation, int pos, ColocationSetType &consequents, ColocationType &tmp);

    // Generate R_(k+1)
    void _generateRules(unsigned int k);
public:
    friend bool isRReachable(LocationType &, LocationType &);

    std::set<RuleType> execute();
};


#endif //JOINBASE_JOINBASE_H
