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
using std::cerr;

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
	vector<int> uniqRouters;
	vector<Route> routes;
	vector<Router> routers;

	void readFile(ifstream &inFile);
	void createRouters();
	void routerSpinUp();
};

#endif //LINKSTATE_MANAGER_H
