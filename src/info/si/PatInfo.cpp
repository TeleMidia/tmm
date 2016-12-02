/******************************************************************************
TeleMídia Multiplexer
Copyright (C) 2016 TeleMídia Lab/PUC-Rio
https://github.com/TeleMidia/tmm-mpeg2ts/graphs/contributors

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU Affero General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version. This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
for more details. You should have received a copy of the GNU Affero General
Public License along with this program. If not, see http://www.gnu.org/licenses/.

*******************************************************************************/

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
