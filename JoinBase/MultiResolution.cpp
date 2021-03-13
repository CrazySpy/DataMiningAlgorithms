//
// Created by 蒋希文 on 2021/2/3.
//

#include <set>
#include "MultiResolution.h"

extern bool multi_rel(CellPositionType &, CellPositionType &);

MultiResolution::MultiResolution(std::vector<InstanceType> &instances, double min_pre, double cellSize)
        : _min_pre(min_pre),
          _cellSize(cellSize) {
    for(auto &instance : instances) {
        auto feature = std::get<Feature>(instance);
        auto id = std::get<Id>(instance);
        auto location = std::get<Location>(instance);

        double realX = location.first, realY = location.second;
        int x = realX / _cellSize, y = realY / cellSize;

        _instances[{x, y}][feature].push_back(id);
        _nInstancesOfFeature[feature]++;

        _tableInstances[1][{feature}].push_back({{x, y}});
    }
}

ColocationSetType MultiResolution::_selectPrevalentColocations(MultiResolution_ColocationPackage &colocationPackages) {
    ColocationSetType prevalence;

    for(auto it = colocationPackages.cbegin(); it != colocationPackages.cend(); ++it) {
        const ColocationType &candidate = (*it).first;
        const MultiResolution_TableInstanceType &tableInstance = (*it).second;

        bool isPrevalent = true;
        for(unsigned int i = 0; i < candidate.size(); ++i) {
            FeatureType feature = candidate[i];
            int cnt = 0; // The number of feature instances in the cells.
            std::set<CellPositionType> counted; // Record whether the instances in the cell are counted.
            for(auto &rowInstance : tableInstance) {
                auto &cellPosition = rowInstance[i];
                if(!counted.count(cellPosition)) {
                    counted.insert(cellPosition);
                    cnt += _instances[cellPosition][feature].size();
                }
            }

            if(cnt * 1.0 / _nInstancesOfFeature[feature] < _min_pre) {
                isPrevalent = false;
                break;
            }
        }

        if (isPrevalent) {
            unsigned int k = colocationPackages.begin()->first.size();
            _tableInstances[k][candidate] = tableInstance;
            prevalence.push_back(candidate);
        }
    }

    return prevalence;
}

MultiResolution_ColocationPackage MultiResolution::_generateTableInstances(ColocationSetType &candidates, int k) {
    // Generate new multi-resolution table instances.
    MultiResolution_ColocationPackage colocationPackages;

    for(auto &candidate : candidates) {
        ColocationType colocation1(candidate.begin(), candidate.end() - 1);
        ColocationType colocation2(candidate.begin(), candidate.end() - 2);
        colocation2.push_back(candidate.back());

        MultiResolution_TableInstanceType &tableInstance1 = _tableInstances[k][colocation1],
                &tableInstance2 = _tableInstances[k][colocation2];


        // Merge two row instances if they are able to merge.
        for (auto it1 = tableInstance1.begin(); it1 != tableInstance1.end(); ++it1) {
            MultiResolution_RowInstanceType &rowInstance1 = *it1;
            for (auto it2 = tableInstance2.begin(); it2 != tableInstance2.end(); ++it2) {
                MultiResolution_RowInstanceType &rowInstance2 = *it2;

                bool canMerge = true;
                for (unsigned int idx = 0; idx < k - 1; ++idx) {
                    if (rowInstance1[idx] != rowInstance2[idx]) {
                        canMerge = false;
                        break;
                    }
                }

                if (canMerge) {
                    CellPositionType &cell1 = rowInstance1.back(),
                            &cell2 = rowInstance2.back();

                    if (multi_rel(cell1, cell2)) {
                        MultiResolution_RowInstanceType newRowInstance(rowInstance1.begin(), rowInstance1.end() - 1);
                        if (colocation1.back() < colocation2.back()) {
                            newRowInstance.push_back(rowInstance1.back());
                            newRowInstance.push_back(rowInstance2.back());
                        } else {
                            newRowInstance.push_back(rowInstance2.back());
                            newRowInstance.push_back(rowInstance1.back());
                        }

                        colocationPackages[candidate].push_back(std::move(newRowInstance));
                    }
                }
            }
        }
    }

    return colocationPackages;
}

void MultiResolution::multiResolutionPruning(ColocationSetType &candidates, int k) {
    auto colocationPackages = _generateTableInstances(candidates, k);
    candidates =  _selectPrevalentColocations(colocationPackages);
}
