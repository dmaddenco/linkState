//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#ifndef LINKSTATE_MANAGER_H
#define LINKSTATE_MANAGER_H

#include "router.h"

#include <iostream>

using std::cout;
using std::endl;
using std::cin;

#include <fstream>

using std::ifstream;

#include <string>

using std::string;

#include <vector>

using std::vector;
using std::find;

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

class Manager {
public:
	void readFile(ifstream &inFile);
	vector<int> routers;
};

#endif //LINKSTATE_MANAGER_H
