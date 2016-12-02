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
 * SdtInfo.cpp
 *
 *  Created on: 16/04/2014
 *      Author: Felippe Nagato
 */

#include "info/si/SdtInfo.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

SdtInfo::SdtInfo() {

}

SdtInfo::~SdtInfo() {

}

bool SdtInfo::printTable() {
	vector<ServiceInformation*>::iterator it;
	vector<MpegDescriptor*>::iterator itDesc;
	Service* service;

	cout << "SDT - Service Descriptor Table" << endl <<
			"------------------------------" << endl << endl;

	if (!serviceList.size()) {
		cout << "No service descriptors available." << endl;
		return false;
	}

	it = serviceList.begin();
	while (it != serviceList.end()) {
		cout << "Service ID " << (*it)->serviceId << ":" << endl;
		itDesc = (*it)->descriptorList.begin();
		while (itDesc != (*it)->descriptorList.end()) {
			switch ((*itDesc)->getDescriptorTag()) {
			case 0x48:
				service = (Service*)(*itDesc);
				cout << "Service name: " << service->getServiceName() << endl;
				cout << "Provider name: " << service->getProviderName() << endl;
				cout << "Service type: ";
				switch (service->getServiceType()) {
				case DIGITAL_TELEVISION_SERVICE:
					cout << "Digital television service";
					break;
				case DIGITAL_AUDIO_SERVICE:
					cout << "Digital audio service";
					break;
				case DATA_BROADCASTING_SERVICE:
					cout << "Data broadcasting service";
					break;
				case DATA_SERVICE:
					cout << "Data service";
					break;
				default:
					cout << " - Unknown service";
					break;
				}
				cout << endl;
				break;
			}
			++itDesc;
		}
		cout << "EIT present and following: ";
		if ((*it)->eitPresentFollowingFlag) cout << "Yes"; else cout << "No";
		cout << endl;
		cout << "EIT schedule: ";
		if ((*it)->eitScheduleFlag) cout << "Yes"; else cout << "No";
		cout << endl;
		cout << "Free CA mode: ";
		if ((*it)->freeCaMode) cout << "Yes"; else cout << "No";
		cout << endl;
		cout << "Running status: ";
		switch ((*it)->runningStatus) {
		case 0x00:
			cout << "Undefined";
			break;
		case 0x01:
			cout << "Not running";
			break;
		case 0x02:
			cout << "Starting in a few seconds";
			break;
		case 0x03:
			cout << "Pausing";
			break;
		case 0x04:
			cout << "Running";
			break;
		}
		cout << endl;
		++it;
		if (it != serviceList.end()) cout << endl;
	}

	return true;
}

}
}
}
}


