/*
 * PIIP.cpp
 *
 *  Created on: 26/09/2013
 *      Author: Felippe Nagato
 */

#include "project/isdbt/PIIP.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

PIIP::PIIP() {
	init();
}

PIIP::PIIP(int id) {
	this->id = id;
	init();
}

PIIP::~PIIP() {

}

void PIIP::init() {
	projectType = PT_IIP;
	layer = 0x08; //IIP
}

int PIIP::encode(int64_t stc, vector<pair<char*,int>*>* list) {
	pair<char*,int>* myp = new pair<char*,int>;

	updateStream();

	myp->first = stream;
	myp->second = 184;

	list->push_back(myp);

	if (mcci) {
		if (mcci->getTMCCSynchronizationWord() == 0) {
			mcci->setTMCCSynchronizationWord(1);
		} else {
			mcci->setTMCCSynchronizationWord(0);
		}
	}

	return 0;
}

}
}
}
}


