#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>
#include "CPITree.h"
#include "Types.h"
#include <chrono>

#include "CSVReader/CSVReader.h"

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

using namespace std;

double maxDistance;

bool isNeighbourhood(const InstanceType &instance1, const InstanceType &instance2) {
    return pow(instance1.location.first - instance2.location.first, 2) + pow(instance1.location.second - instance2.location.second, 2) <= maxDistance * maxDistance;
}

int main(int argc, char **argv) {
    if(argc != 3) {
        cout << "Argument number must be 2" << endl;
        cout << "./CPITree maximum_neighbourhood_distance inputPath" << endl;
        return 0;
    }
    maxDistance = stod(argv[1]);
    string inputPath(argv[2]);

    CSVReader csvReader(inputPath);

    std::vector<InstanceType> instances;

    while(csvReader.hasNext()) {
        auto line = csvReader.getNextRecord();

        unsigned char feature = line[0][0];
        unsigned int id = stoul(line[1]);
        double x = stod(line[2]), y = stod(line[3]);

        instances.push_back({feature, id, make_pair(x, y)});

    }
    cout << instances.size() << endl;

    high_resolution_clock::time_point beginTime = high_resolution_clock::now();

    CPITree cpiTree(instances);
    auto cliques = cpiTree.execute();

    high_resolution_clock::time_point endTime = high_resolution_clock::now();
    milliseconds timeInterval = std::chrono::duration_cast<milliseconds>(endTime - beginTime);

    for(auto &clique : cliques) {
        for(auto &instance : clique) {
            cout << instance.first << '.' << instance.second << ' ';
        }
        cout << endl;
    }

    std::cout << timeInterval.count() << "ms\n";

    return 0;
}
/*
 *  neighbourhood threshold : sqrt(3)
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