//
// Created by 蒋希文 on 2021/3/14.
//

#include "FractionScore.h"

FractionScore::FractionScore(const std::vector<InstanceType> &instances,
                             const std::vector<std::pair<InstanceNameType , InstanceNameType>> &neighbourhoods,
                             const std::vector<std::pair<InstanceNameType , InstanceNameType>> &neighbourhoods2) {
    // Fraction computation.
    for(auto &neighbourhood : neighbourhoods) {
        auto &instance1 = neighbourhood.first, &instance2 = neighbourhood.second;

        if (instance1.first == instance2.first) continue;

        _disk[instance1].insert(instance2.first);
        _disk[instance2].insert(instance1.first);

        _neighbourhoodCount[{instance1, instance2.first}]++;
        _neighbourhoodCount[{instance2, instance1.first}]++;

    }

    for(auto &neighbourhood : neighbourhoods) {
        auto &instance1 = neighbourhood.first, &instance2 = neighbourhood.second;
        if (instance1.first == instance2.first) continue;

        auto &neighbourhoodCount1 = _neighbourhoodCount[{instance1, instance2.first}];
        auto &neighbourhoodCount2 = _neighbourhoodCount[{instance2, instance1.first}];

        auto &labelVal1 = _labelVal[{instance1, instance2.first}];
        auto &labelVal2 = _labelVal[{instance2, instance1.first}];

        labelVal1 += 1.0 / neighbourhoodCount2;
        labelVal2 += 1.0 / neighbourhoodCount1;

        if(labelVal1 > 1) {
            labelVal1 = 1;
        }
        if(labelVal2 > 1) {
            labelVal2 = 1;
        }
    }

    _maxFeatureInstanceCount = 0;
    // Construct "_instances" variable.
    for(auto &instance : instances) {
        _instances[std::get<Feature>(instance)].push_back(std::get<Id>(instance));
        _maxFeatureInstanceCount = std::max<unsigned int>(_maxFeatureInstanceCount, _instances[std::get<Feature>(instance)].size());
    }

    // Construct "_disk2" variable.
    for(auto &neighbourhood : neighbourhoods2) {
        auto &instance1 = neighbourhood.first, &instance2 = neighbourhood.second;
        if(instance1.first == instance2.first) continue;

        _disk2[instance1].insert(instance2.first);
        _disk2[instance2].insert(instance1.first);
    }
}

double FractionScore::_fractionAggregation(const ColocationType &candidate,
                                           const InstanceNameType &instance) {
    double labelSetVal = -1;
    for(auto feature : candidate) {
        if(feature == instance.first) continue;

        double labelVal = _labelVal[{instance, feature}];
        if(labelSetVal > labelVal || labelSetVal < 0) {
            labelSetVal = labelVal;
        }
    }

    return labelSetVal < 0 ? 0 : labelSetVal;
}

bool FractionScore::_isInstanceInTableInstance(const ColocationType &candidate,
                                               const InstanceNameType &instance,
                                               const std::vector<std::vector<InstanceNameType>> &tableInstance) {
    // Filter 1.
    if(_existedInTableInstance.count(candidate) && _existedInTableInstance[candidate].count(instance)) {
        return true;
    }

    // Filter 2.
    auto &featuresOnDisk = _disk[instance];
    for(auto feature : candidate) {
        if(instance.first == feature) continue;
        if(!featuresOnDisk.count(feature)) {
            return false;
        }
    }

    // Filter 3.
    auto &featuresOnDisk2 = _disk2[instance];
    bool allIn = true;
    for(auto feature : candidate) {
        if(instance.first == feature) continue;
        if(!featuresOnDisk2.count(feature)) {
            allIn = false;
            break;
        }
    }
    if(allIn) return true;

    // Other
    for(auto &rowInstance : tableInstance) {
        for(unsigned int i = 0; i < rowInstance.size(); ++i) {
            _existedInTableInstance[candidate].insert(rowInstance[i]);

            if(instance == rowInstance[i]) {
                return true;
            }
        }
    }

    return false;
}

double FractionScore::_conditionalSupportComputation(const ColocationType &candidate,
                                                     const std::vector<std::vector<InstanceNameType>> &tableInstance,
                                                     const FeatureType feature) {
    // sup(C|feature).
    double supp = 0;
    if(candidate.size() == 1 && candidate[0] == feature) {
        supp = tableInstance.size();
    } else {
        for (auto instanceId : _instances[feature]) {
            if (_isInstanceInTableInstance(candidate, {feature, instanceId}, tableInstance)) {
                supp += _fractionAggregation(candidate, {feature, instanceId});
            }
        }
    }

    return supp;
}

double FractionScore::supportComputation(const ColocationType &candidate,
                                         const std::vector<std::vector<InstanceNameType>> &tableInstance) {


    double supp = -1;
    for(auto feature : candidate) {
        double conditionalSupp = _conditionalSupportComputation(candidate, tableInstance, feature);
        if(conditionalSupp < supp || supp < 0) {
            supp = conditionalSupp;
        }
    }
    return supp / _maxFeatureInstanceCount;
}

double FractionScore::confComputation(
        const ColocationType &colocation,
        const ColocationType &antecedent,
        const std::vector<std::vector<InstanceNameType>> &colocationTableInstance,
        const std::vector<std::vector<InstanceNameType>> &antecedentTableInstance) {
    double conf = -1;
    for(auto feature : antecedent) {
        double t1 = _conditionalSupportComputation(colocation, colocationTableInstance, feature);
        double t2 = _conditionalSupportComputation(antecedent, antecedentTableInstance, feature);

        if(t1 / t2 < conf || conf < 0) {
            conf = t1 / t2;
        }
    }

    return conf;
}

void FractionScore::clearExistedInTableInstance() {
    _existedInTableInstance.clear();
}