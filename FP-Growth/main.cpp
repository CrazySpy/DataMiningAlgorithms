#include "FPTree.h"
#include "FPGrowth.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

using namespace std;

int main(int nArg, char *args[]) {
    if(nArg != 3) {
        cout << "Argument number must be 2" << endl;
        cout << "./FPGrowth supportRage inputPath" << endl;
        return 0;
    }
    double supp = stod(args[1]);
    string inputPath(args[2]);

    //double supp = 0.2;
    //string inputPath("/Users/jiangxiwen/Desktop/Data mining/mushroom.dat");

    ifstream fs(inputPath, ios::in);
    TransSet<string> trans = TransSet<string>();

    string line;
    while(getline(fs, line))
    {
        if(line.back() == '\n') line.pop_back(); // pop back the enter.
        ItemSet<string> itemset;

        stringstream ss(line);
        string item;
        while(ss >> item)
        {
            itemset.push_back(move(item));
        }

        sort(itemset.begin(), itemset.end());
        trans.push_back(move(itemset));
    }

    /*
    TransSet<int> trans = TransSet<int>();
    double supp = 0.2;

    trans.push_back({2, 1, 5});
    trans.push_back({2, 1, 3, 5});
    trans.push_back({2, 1, 3});
    trans.push_back({2, 1, 4});
    trans.push_back({2, 3});
    trans.push_back({2, 3});
    trans.push_back({2, 4});
    trans.push_back({1, 3});
    trans.push_back({1, 3});
     */
    freopen("output.txt", "w", stdout);

    high_resolution_clock::time_point beginTime = high_resolution_clock::now();
    FPGrowth<string> fp(trans, supp);

    auto freqSet = fp.execute();

    high_resolution_clock::time_point endTime = high_resolution_clock::now();
    milliseconds timeInterval = std::chrono::duration_cast<milliseconds>(endTime - beginTime);

    for(auto &s : *freqSet) {
        for(auto &item : s.first) {
            cout << item << ' ';
        }
        cout << "with support " << s.second * 1.0 / trans.size() << endl;
    }

   std::cout << timeInterval.count() << "ms\n";

    return 0;
}