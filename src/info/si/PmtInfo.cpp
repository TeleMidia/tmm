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
 * PmtInfo.cpp
 *
 *  Created on: 16/04/2014
 *      Author: Felippe Nagato
 */

#include "info/si/PmtInfo.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

PmtInfo::PmtInfo() {
	pid = 0;
}

PmtInfo::~PmtInfo() {

}

void PmtInfo::setPid(unsigned short pid) {
	this->pid = pid;
}

bool PmtInfo::printTable() {
	Pmt* pmt;
	map<unsigned short, unsigned char>::iterator k;

	cout << "PMT - Program Map Table" << endl <<
			"-----------------------" << endl;

	if (esList != NULL) {
		cout << endl;
		if (pid == 16) {
			cout << "Network PMT entry on PID " << pid << "." << endl;
		} else {
			cout << "Service with program number " << tableIdExtension <<
					" and PCR on PID " << pcrPid << ":" << endl;
			k = esList->begin();
			while (k != esList->end()) {
				cout << "ES PID: " << k->first << " - ";
				cout << pmt->getStreamTypeName(k->second) << endl;
				++k;
			}
		}
	}

	return true;
}

}
}
}
}


