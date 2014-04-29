/*
 * PatInfo.cpp
 *
 *  Created on: 16/04/2014
 *      Author: Felippe Nagato
 */

#include "info/si/PatInfo.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

PatInfo::PatInfo() {

}

PatInfo::~PatInfo() {

}

bool PatInfo::printTable() {
	map<unsigned short, unsigned short>::iterator it;

	cout << "PAT - Program Association Table" << endl <<
			"-------------------------------" << endl << endl;
	if (pmtList->size()) {
		it = pmtList->begin();
		while (it != pmtList->end()) {
			if (it->second != 0) {
			cout << "PMT: PID " << it->first << " - Program number " <<
					it->second << endl;
			} else {
				cout << "PMT: PID " << it->first << " - Network" << endl;
			}
			++it;
		}
	} else {
		cout << "No programs available." << endl;
		return false;
	}

	return true;
}

}
}
}
}
