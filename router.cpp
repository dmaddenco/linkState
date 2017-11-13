//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#include "router.h"

int Router::getAddr() {
	return this->ownAddr;
}

void Router::setAddr(int addr) {
	this->ownAddr = addr;
}

int main(int argc, char *argv[]) {
	vector<Router> allRouters;
	for (int i = 0; i < 4; ++i) {
		Router router;
		router.setAddr(i);
		allRouters.push_back(router);
	}
	for (int j = 0; j < allRouters.size(); ++j) {
		cout << allRouters[j].getAddr() << endl;
	}
}