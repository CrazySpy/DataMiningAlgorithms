#include <iostream>
#include "JoinLess.h"
#include <cmath>
#include <tuple>
#include <chrono>
#include <string>

#include "CSVReader/CSVReader.h"

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

using namespace std;

double maxDistance;

bool hasRelation(const InstanceType &instance1, const InstanceType &instance2) {
    auto loc1 = std::get<Location>(instance1);
    auto loc2 = std::get<Location>(instance2);

    return pow(loc1.first - loc2.first, 2) + pow(loc1.second - loc2.second, 2) <= maxDistance * maxDistance;
}

int main(int argc, char **argv) {
    if(argc != 5) {
        cout << "Argument number must be 4" << endl;
        cout << argv[0] << " minimum_prevalence minimum_rule_probability maximum_neighbourhood_distance inputPath" << endl;
        return 0;
    }
    double minPI = stod(argv[1]), minRuleProbability = stod(argv[2]);
    maxDistance = stod(argv[3]);
    string inputPath(argv[4]);

    CSVReader csvReader(inputPath);

    std::vector<InstanceType> instances;

    while(csvReader.hasNext()) {
        auto line = csvReader.getNextRecord();

        FeatureType feature = line[0];
        InstanceIdType id = stoul(line[1]);
        double x = stod(line[2]), y = stod(line[3]);

        instances.emplace_back(make_tuple(feature, id, make_pair(x, y)));
    }
    cout << instances.size() << endl;

    high_resolution_clock::time_point beginTime = high_resolution_clock::now();

    JoinLess joinLess(instances, minPI, minRuleProbability);
    set<RuleType> rules = joinLess.execute();

    high_resolution_clock::time_point endTime = high_resolution_clock::now();
    milliseconds timeInterval = std::chrono::duration_cast<milliseconds>(endTime - beginTime);

    for(auto &rule : rules) {
        const ColocationType &colocation1 = rule.first, &colocation2 = rule.second;
        for(unsigned int i = 0; i < colocation1.size(); ++i) {
            cout << colocation1[i] << ' ';
        }

        cout << "=> ";

        for(unsigned int i = 0; i < colocation2.size(); ++i) {
            cout << colocation2[i] << ' ';
        }
        cout << endl;
    }

    std::cout << timeInterval.count() << "ms\n";

    return 0;
}
/*
    instances.push_back({1, 'A', {0, 2}});
    instances.push_back({2, 'A', {11, 3}});
    instances.push_back({3, 'A', {12,3}});
    instances.push_back({4, 'A', {10, 4}});

    instances.push_back({1, 'B', {0, 1}});
    instances.push_back({2, 'B', {13,1.5}});
    instances.push_back({3, 'B', {20, 5}});
    instances.push_back({4, 'B', {11,2}});
    instances.push_back({5, 'B', {31,11}});

    instances.push_back({1, 'C', {12,1.5}});
    instances.push_back({2, 'C', {0, 3}});
    instances.push_back({3, 'C', {30, 10}});
 */