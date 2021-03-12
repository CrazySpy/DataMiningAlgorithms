#include "Apriori.h"
#include "HashTree.h"
#include <string>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <sstream>
#include <algorithm>
#include <chrono>

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

using namespace std;


int main(int argc, char **argv) {
	if(argc != 4) {
		cout << "Argument number must be 3" << endl;
		cout << "./Apriori supportRank confidenceRank inputPath" << endl;
		return 0;
	}
	double supp = stod(argv[1]), conf = stod(argv[2]);
	string inputPath(argv[3]);

	ifstream fs(inputPath, ios::in);

	std::shared_ptr<ItemSetSet<string>> transSet = std::make_shared<ItemSetSet<string>>();
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
		transSet->push_back(move(itemset));
	}

	/*
	std::shared_ptr<ItemSetSet<string>> transSet = std::make_shared<ItemSetSet<string>>();
	transSet->push_back(ItemSet<string>{"I1", "I2", "I5"});
	transSet->push_back(ItemSet<string>{"I2", "I4"});
	transSet->push_back(ItemSet<string>{"I2", "I3"});
	transSet->push_back(ItemSet<string>{"I1", "I2", "I4"} );
	transSet->push_back(ItemSet<string>{"I1", "I3"});
	transSet->push_back(ItemSet<string>{"I2", "I3"});
	transSet->push_back(ItemSet<string>{"I1", "I3"});
	transSet->push_back(ItemSet<string>{"I1", "I2", "I3", "I5"});
	transSet->push_back(ItemSet<string>{"I1", "I2", "I3"});
	*/

    high_resolution_clock::time_point beginTime = high_resolution_clock::now();

    Apriori<string> apriori(transSet, supp, conf);
    auto rules = apriori.execute();


    high_resolution_clock::time_point endTime = high_resolution_clock::now();
    milliseconds timeInterval = std::chrono::duration_cast<milliseconds>(endTime - beginTime);

    for(auto &rule : *rules)
    {
        for(auto &item : rule.antecedent) cout << item << ' ';
        
        cout << " => ";
        
        for(auto &item : rule.consequent) cout << item << ' ';

		cout << "with confidence rate : " << rule.conf;

        cout << endl;
    }

    std::cout << timeInterval.count() << "ms\n";
    return 0;
}
