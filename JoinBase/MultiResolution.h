//
// Created by 蒋希文 on 2021/2/3.
//

#ifndef JOINBASE_MULTIRESOLUTION_H
#define JOINBASE_MULTIRESOLUTION_H

#include "Types.h"
#include <map>

class MultiResolution {
private:
    double _min_pre;
    double _cellSize;

    std::map<FeatureType, unsigned int> _nInstancesOfFeature;

    std::map<CellPositionType, std::map<FeatureType, std::vector<InstanceIdType>>> _instances;

    std::map<unsigned int, std::map<ColocationType, MultiResolution_TableInstanceType>> _tableInstances;

public:
    friend bool multi_rel(CellPositionType &, CellPositionType &);

    MultiResolution(std::vector<InstanceType> &instances, double min_pre, double cellSize);

    void multiResolutionPruning(ColocationSetType &candidates, int k);

private:
    MultiResolution_ColocationPackage _generateTableInstances(ColocationSetType &candidates, int k);

    ColocationSetType _selectPrevalentColocations(MultiResolution_ColocationPackage &colocationPackages);
};


#endif //JOINBASE_MULTIRESOLUTION_H
