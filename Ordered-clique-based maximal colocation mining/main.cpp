#include <iostream>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "Types.h"
#include <chrono>

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

using namespace std;

double maxDistance;

bool isNeighbourhood(const InstanceType &instance1, const InstanceType &instance2) {
    return pow(instance1.location.first - instance2.location.first, 2) + pow(instance1.location.second - instance2.location.second, 2) <= maxDistance * maxDistance;
}

extern std::vector<std::vector<FeatureType>> generatePrevalentColocations(const std::vector<InstanceType> &instances,
                                                                          double minPre);


int main(int argc, char **argv) {
    if(argc != 4) {
        cout << "Argument number must be 3" << endl;
        cout << "./CPITreeAdvanced minimum_prevalence maximum_neighbourhood_distance inputPath" << endl;
        return 0;
    }
    double minPre = stod(argv[1]);
    maxDistance = stod(argv[2]);
    string inputPath(argv[3]);

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

        instances.push_back({feature, id, make_pair(x, y)});

    }
    cout << instances.size() << endl;

    high_resolution_clock::time_point beginTime = high_resolution_clock::now();

    auto colocations = generatePrevalentColocations(instances, minPre);

    high_resolution_clock::time_point endTime = high_resolution_clock::now();
    milliseconds timeInterval = std::chrono::duration_cast<milliseconds>(endTime - beginTime);

    for (auto &colocation : colocations) {
        for(auto &feature : colocation) {
            cout << feature << ' ';
        }
        cout << endl;
    }

    std::cout << timeInterval.count() << "ms\n";

    return 0;
}
/*
 *  distance: sqrt(3)
    instances.push_back({ 'A', 1, {0, 2}});
    instances.push_back({ 'A', 2, {11, 3}});
    instances.push_back({ 'A', 3, {12,3}});
    instances.push_back({ 'A', 4,{10, 4}});

    instances.push_back({ 'B', 1, {0, 1}});
    instances.push_back({ 'B', 2, {13,1.5}});
    instances.push_back({ 'B', 3, {20, 5}});
    instances.push_back({ 'B', 4, {11,2}});
    instances.push_back({ 'B', 5, {31,11}});

    instances.push_back({ 'C', 1, {12,1.5}});
    instances.push_back({ 'C', 2, {0, 3}});
    instances.push_back({ 'C', 3, {30, 10}});
 */
