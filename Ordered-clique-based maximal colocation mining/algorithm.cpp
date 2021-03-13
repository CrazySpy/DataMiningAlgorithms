//
// Created by 蒋希文 on 2021/3/10.
//

#include <vector>
#include "Types.h"
#include "PmTree.h"
#include "InsTree.h"

extern bool isNeighbourhood(const InstanceType &instance1, const InstanceType &instance2);

// The function is used to candidates' prevalence according to the bitmap.
bool isBitmapPrevalent(const std::vector<bool> &bitmap,
                 double minPre) {
    unsigned int trueCount = 0;
    for(unsigned int i = 0; i < bitmap.size(); ++i) {
        if(bitmap[i]) {
            trueCount++;
        }
    }

    if(trueCount * 1.0 / bitmap.size() < minPre) {
        return false;
    }
    return true;
}

bool isBitmapPrevalent(const std::vector<std::vector<bool>> &bitmaps,
                 double minPre) {
    for(auto &bitmap : bitmaps) {
        if(!isBitmapPrevalent(bitmap, minPre)) {
            return false;
        }
    }

    return true;
}

bool isBitmapPrevalent(const std::pair<std::vector<bool>, std::vector<bool>> &bitmaps,
                 double minPre) {
    return isBitmapPrevalent(bitmaps.first, minPre) && isBitmapPrevalent(bitmaps.second, minPre);
}

std::vector<std::pair<FeatureType, FeatureType>> generateSize2PrevalentColocations(const std::vector<std::pair<InstanceNameType, InstanceNameType>> &neighbourhoods,
                                                                            std::map<FeatureType, std::vector<InstanceIdType>> &instancesCount,
                                                                            double minPre) {
    std::map<std::pair<FeatureType, FeatureType>, std::pair<std::vector<bool>, std::vector<bool>>> bitmaps;
    for(auto &neighbourhood : neighbourhoods) {
        auto &instance1 = neighbourhood.first, &instance2 = neighbourhood.second;

        if(instance1.first == instance2.first) continue;

        unsigned int feature1Count = instancesCount[instance1.first].size(),
                     feature2Count = instancesCount[instance2.first].size();
        std::pair<FeatureType, FeatureType> featurePair = {instance1.first, instance2.first};

        if(!bitmaps.count(featurePair)) {
            bitmaps[featurePair] = std::make_pair(std::vector<bool>(feature1Count, false), std::vector<bool>(feature2Count, false));
        }

        auto &bitmap = bitmaps[featurePair];
        (bitmap.first)[instance1.second - 1] = true;
        (bitmap.second)[instance2.second - 1] = true;
    }

    std::vector<std::pair<FeatureType , FeatureType>> size2PrevalentColocation;
    for(auto it = bitmaps.begin(); it != bitmaps.end(); ++it) {
        auto &bitmap = (*it).second;

        if(isBitmapPrevalent(bitmap, minPre)) {
            auto &neighbourhood = (*it).first;
            size2PrevalentColocation.push_back(neighbourhood);
        }
    }

    return size2PrevalentColocation;
}

bool isCandidatePrevalent(const std::vector<FeatureType> &candidate,
                          double minPre,
                          const std::vector<std::pair<InstanceNameType, InstanceNameType>> &neighbourhoods,
                          std::map<FeatureType, std::vector<InstanceIdType>> &instancesCount) {
    InsTree insTree(neighbourhoods, candidate);

    auto tableInstance = insTree.generateInstances();

    if (tableInstance.empty()) return false;

    std::map<FeatureType, std::vector<bool>> bitmap;
    for(auto feature : candidate) {
        bitmap[feature] = std::vector<bool>(instancesCount[feature].size(), false);
    }
    for(auto it = tableInstance.begin(); it != tableInstance.end(); ++it) {
        auto &rowInstance = (*it);
        for(unsigned int i = 0; i < rowInstance.size(); ++i) {
            bitmap[candidate[i]][rowInstance[i].second - 1] = true;
        }
    }

    // If the variable 'isPrevalent' set to false, it means the candidate is not prevalent,
    // we need to check whether the subset is prevalent.
    bool isPrevalent = true;
    for(auto it = bitmap.begin(); it != bitmap.end(); ++it) {
        if(!isBitmapPrevalent((*it).second, minPre)) {
            isPrevalent = false;
            break;
        }
    }

    return isPrevalent;
}

void generateSubsets(const std::vector<FeatureType> &candidates,
                     int remainder,
                     int k,
                     std::vector<std::vector<FeatureType>> &subsets,
                     std::vector<FeatureType> &tmp) {
    if(!remainder) {
        subsets.push_back(tmp);
        return;
    }

    if(k + remainder > candidates.size()) {
        return;
    }

    tmp.push_back(candidates[k]);
    generateSubsets(candidates, remainder - 1, k + 1, subsets, tmp);
    tmp.pop_back();

    generateSubsets(candidates, remainder, k + 1, subsets, tmp);
}

std::vector<std::vector<FeatureType>> generatePrevalentColocations(const std::vector<InstanceType> &instances,
                                                                       double minPre) {
    std::vector<std::pair<InstanceNameType, InstanceNameType>> neighbourhoods;

    std::map<FeatureType, std::vector<InstanceIdType>> instancesCount;
    for(auto it1 = instances.begin(); it1 != instances.end(); ++it1) {
        instancesCount[(*it1).feature].push_back((*it1).id);

        for(auto it2 = it1 + 1; it2 != instances.end(); ++it2) {
            if(isNeighbourhood(*it1, *it2)) {
                neighbourhoods.push_back({{(*it1).feature, (*it1).id}, {(*it2).feature, (*it2).id}});
            }
        }
    }

    auto size2PrevalentColocations = generateSize2PrevalentColocations(neighbourhoods, instancesCount, minPre);

    // The steps finish generating P2Tree, PmTree and candidates.
    PmTree pmTree = PmTree(size2PrevalentColocations);
    auto candidates = pmTree.generateCandidate();
    // Generate finished.

    std::vector<std::vector<FeatureType>> prevalent;
    // Store the checked colocations.
    std::set<std::vector<FeatureType>> checked;

    for(auto &candidate : candidates) {
        if(isCandidatePrevalent(candidate, minPre, neighbourhoods, instancesCount)) {
            prevalent.push_back(candidate);
            continue;
        }

        // Otherwise, check whether the subset is prevalent.
        std::vector<std::vector<FeatureType>> newCandidates;
        std::vector<FeatureType> tmp;
        bool found = false;
        for(int i = candidate.size() - 1; i >= 2; --i) {
            generateSubsets(candidate, i, 0, newCandidates, tmp);

            for (auto &candidate : newCandidates) {
                if (checked.count(candidate)) continue;

                if (isCandidatePrevalent(candidate, minPre, neighbourhoods, instancesCount)) {
                    prevalent.push_back(candidate);
                    found = true;
                }

                checked.insert(candidate);
            }

            if (found) break;
        }
    }

    return prevalent;
}