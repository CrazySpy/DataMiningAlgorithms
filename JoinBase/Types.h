//
// Created by 蒋希文 on 2021/1/31.
//

#ifndef JOINBASE_TYPES_H
#define JOINBASE_TYPES_H

#include <iostream>
#include <vector>
#include <map>
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

// It's defined for containing Colocation set and corresponding table instances.
//using ColocationPackage = std::map<ColocationSetType, TableInstanceType>;
using ColocationPackage = std::map<ColocationType, TableInstanceType>;

using RuleType = std::pair<ColocationType, ColocationType>;

InstanceType make_instance(InstanceIdType &instanceId, FeatureType &feature, LocationType &location);
ColocationSetPtrType make_colocationSet();


/*
 * MultiResolution Part
 */
using CellPositionType = std::pair<int, int>;
using MultiResolution_RowInstanceType = std::vector<CellPositionType>;
using MultiResolution_TableInstanceType = std::vector<MultiResolution_RowInstanceType>;
using MultiResolution_ColocationPackage = std::map<ColocationType, MultiResolution_TableInstanceType>;

#endif //JOINBASE_TYPES_H
