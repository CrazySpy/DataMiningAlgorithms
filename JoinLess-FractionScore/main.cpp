#include <iostream>
#include "JoinLess.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <tuple>
#include <chrono>

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

using namespace std;

int main(int argc, char **argv) {
    if(argc != 5) {
        cout << "Argument number must be 4" << endl;
        cout << argv[0] << " minimum_prevalence minimum_rule_probability maximum_neighbourhood_distance inputPath" << endl;
        return 0;
    }
    double minPre = stod(argv[1]), minRuleProbability = stod(argv[2]);
    double maxDistance = stod(argv[3]);
    string inputPath(argv[4]);

    ifstream ifs(inputPath, ios::in);

    std::vector<InstanceType> instances;

    std::string line;
    while(getline(ifs, line)) {
        for(int i = 0; i < line.size(); ++i) {
            if(line[i] == ',') {
                line[i] = ' ';
            }
        }

        stringstream ss(line);
        unsigned char feature;
        unsigned int id;
        double x, y;
        ss >> feature >> id >> x >> y;

        instances.emplace_back(make_tuple(id, feature, make_pair(x, y)));
    }
    cout << instances.size() << endl;

    high_resolution_clock::time_point beginTime = high_resolution_clock::now();

    JoinLess joinLess(instances, minPre, minRuleProbability, maxDistance);
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
 * Distance : 1
    instances.push_back({1, 'A', {1, 3}});
    instances.push_back({2, 'A', {0, 2}});
    instances.push_back({3, 'A', {2, 2}});
    instances.push_back({4, 'A', {1, 1}});
    instances.push_back({5, 'A', {10, 3}});
    instances.push_back({6, 'A', {12, 3}});
    instances.push_back({7, 'A', {16, 3}});
    instances.push_back({8, 'A', {19, 3}});
    instances.push_back({9, 'A', {5,  2}});


    instances.push_back({1, 'B', {1, 2}});
    instances.push_back({2, 'B', {5, 3}});
    instances.push_back({3, 'B', {4, 2}});
    instances.push_back({4, 'B', {6, 2}});
    instances.push_back({5, 'B', {5, 1}});
    instances.push_back({6, 'B', {24, 1}});
    instances.push_back({7, 'B', {27, 1}});
    instances.push_back({8, 'B', {29, 1}});
    instances.push_back({9, 'B', {35, 1}});
 */