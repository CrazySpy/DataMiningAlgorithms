//
// Created by 蒋希文 on 2021/3/14.
//

#ifndef JOINLESS_FRACTIONSCORE_H
#define JOINLESS_FRACTIONSCORE_H

#include "Types.h"

class FractionScore {
private:
    std::map<FeatureType, std::vector<InstanceIdType>> _instances;

    unsigned int _maxFeatureInstanceCount;

    // Neigh(o, t, d);
    std::map<std::pair<InstanceNameType, FeatureType>, unsigned int> _neighbourhoodCount;

    // disk(o, d) like variable.
    // But in the article, the "disk" stores the instances.
    // The "_disk" variable stores the feature.
    std::map<InstanceNameType, std::set<FeatureType>> _disk;

    // disk(o, d/2);
    std::map<InstanceNameType, std::set<FeatureType>> _disk2;

    // △label.
    // The variable records the instances' fractions from other features.
    std::map<std::pair<InstanceNameType, FeatureType>, double> _labelVal;

    // The variable is for the filtering approach for 'RI' problem.
    std::map<ColocationType, std::set<InstanceNameType>> _existedInTableInstance;

public:
    // The variable "neighbourhoods" stores all the pairwise instances whose distance is lower than maxDistance.
    // The variable "neighbourhoods2" stores all the pairwise instances whose distance is lower than half of maxDistance.
    FractionScore(const std::vector<InstanceType> &instances,
                  const std::vector<std::pair<InstanceNameType, InstanceNameType>> &neighbourhoods,
                  const std::vector<std::pair<InstanceNameType, InstanceNameType>> &neighbourhoods2);

    double supportComputation(const ColocationType &candidate,
                              const std::vector<std::vector<InstanceNameType>> &tableInstance);

    double confComputation(const ColocationType &colocation, const ColocationType &antecedent,
                           const std::vector<std::vector<InstanceNameType>> &colocationTableInstance,
                           const std::vector<std::vector<InstanceNameType>> &antecedentTableInstance);

    // The function clears the variable '_existedInTableInstance'.
    void clearExistedInTableInstance();

private:
    double _fractionAggregation(const ColocationType &candidate, const InstanceNameType &instance);

    bool _isInstanceInTableInstance(const ColocationType &candidate, const InstanceNameType &instance,
                                    const std::vector<std::vector<InstanceNameType>> &tableInstance);

    double _conditionalSupportComputation(const ColocationType &candidate,
                                          const std::vector<std::vector<InstanceNameType>> &tableInstance,
                                          const FeatureType feature);

};


#endif //JOINLESS_FRACTIONSCORE_H
