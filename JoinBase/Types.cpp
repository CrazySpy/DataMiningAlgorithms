//
// Created by 蒋希文 on 2021/1/31.
//
#include "Types.h"

InstanceType make_instance(InstanceIdType &instanceId, FeatureType &feature, LocationType &location) {
    return std::make_tuple(instanceId, feature, location);
}