//
// Created by 蒋希文 on 2021/3/8.
//

#ifndef CPITREE_TYPES_H
#define CPITREE_TYPES_H

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <memory>

using InstanceIdType = unsigned int;
using FeatureType = unsigned char;
using LocationType = std::pair<double, double>;
struct InstanceType {
    FeatureType feature;
    InstanceIdType id;
    LocationType location;
};

using InstanceNameType = std::pair<FeatureType, InstanceIdType>;

using TableInstanceType = std::map<
        std::pair<FeatureType, FeatureType>,
        std::vector<std::vector<InstanceNameType>>>;

#endif //CPITREE_TYPES_H
