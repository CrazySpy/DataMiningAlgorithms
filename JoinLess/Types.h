//
// Created by 蒋希文 on 2021/2/10.
//

#ifndef JOINLESS_TYPES_H
#define JOINLESS_TYPES_H

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <memory>

using InstanceIdType = unsigned int;
using FeatureType = unsigned char;
using LocationType = std::pair<double, double>;
using InstanceType = std::tuple<InstanceIdType, FeatureType, LocationType>;

using RowInstanceType = std::vector<InstanceIdType>;
using TableInstanceType = std::vector<RowInstanceType>;

enum InstanceEnum {
    Id,
    Feature,
    Location
};

using ColocationType = std::vector<FeatureType>;
using ColocationSetType = std::vector<ColocationType>;
using ColocationSetPtrType = std::shared_ptr<ColocationSetType>;

using RuleType = std::pair<ColocationType, ColocationType>;

#endif //JOINLESS_TYPES_H
