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


