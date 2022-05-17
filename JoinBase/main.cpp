#include <iostream>
#include "JoinBase.h"
#include <cmath>
#include <tuple>
#include <chrono>

#include "CSVReader/CSVReader.h"

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

using namespace std;

double maxDistance;
double cellResolution;

bool multi_rel(CellPositionType &p1, CellPositionType &p2) {
    return abs(p1.first - p2.first) <= 1 && abs(p1.second - p2.second) <= 1;
}

bool isRReachable(LocationType &loc1, LocationType &loc2) {
    return pow(loc1.first - loc2.first, 2) + pow(loc1.second - loc2.second, 2) <= maxDistance * maxDistance;
}

int main(int argc, char **argv) {
    if(argc != 6) {
        cout << "Argument number must be 5" << endl;
        cout << "./JoinBase minimum_prevalence minimum_rule_probability maximum_neighbourhood_distance cell_resolution(0 when turn off) inputPath" << endl;
        return 0;
    }
    double minPI = stod(argv[1]), minRuleProbability = stod(argv[2]);
    maxDistance = stod(argv[3]);
    cellResolution = stod(argv[4]);
    string inputPath(argv[5]);

    CSVReader csvReader(inputPath);

    std::vector<InstanceType> instances;

    while(csvReader.hasNext()) {
        auto line = csvReader.getNextRecord();

        unsigned char feature = line[0][0];
        unsigned int id = stoul(line[1]);
        double x = stod(line[2]), y = stod(line[3]);

        instances.emplace_back(make_tuple(id, feature, make_pair(x, y)));
    }
    cout << instances.size() << endl;

    high_resolution_clock::time_point beginTime = high_resolution_clock::now();

    JoinBase joinBase(instances, minPI, minRuleProbability, cellResolution != 0, cellResolution);
    set<RuleType> rules = joinBase.execute();

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