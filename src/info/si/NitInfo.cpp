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
 * PNit.cpp
 *
 *  Created on: 22/06/2013
 *      Author: Felippe Nagato
 */

#include "info/si/NitInfo.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

NitInfo::NitInfo() {
	regionList[1] = "RO";
	regionList[2] = "AC";
	regionList[3] = "AM";
	regionList[4] = "RR";
	regionList[5] = "PA";
	regionList[6] = "AP";
	regionList[7] = "TO";
	regionList[8] = "MA";
	regionList[9] = "PI";
	regionList[10] = "CE";
	regionList[11] = "RN";
	regionList[12] = "PB";
	regionList[13] = "PE";
	regionList[14] = "SE";
	regionList[15] = "AL";
	regionList[16] = "BA";
	regionList[17] = "MG";
	regionList[18] = "ES";
	regionList[19] = "RJ";
	regionList[20] = "SP";
	regionList[21] = "PR";
	regionList[22] = "SC";
	regionList[23] = "RS";
	regionList[24] = "MS";
	regionList[25] = "MT";
	regionList[26] = "GO";
	regionList[27] = "DF";
}

NitInfo::~NitInfo() {

}

bool NitInfo::printTable() {
	NetworkName* netName;
	TransportInformation* ti;
	vector<MpegDescriptor*>::iterator itDesc;
	vector<TransportInformation*>::iterator itTrans;
	ServiceList* sl;
	map<unsigned short, unsigned char>::iterator itSl;
	TerrestrialDeliverySystem* tds;
	set<unsigned short>::iterator itFl;
	unsigned short value;
	SystemManagement* sm;
	PartialReception* pr;
	TSInformation* tsinfo;

	cout << "NIT - Network Information Table" << endl <<
			"-------------------------------" << endl << endl;
	itDesc = descriptorsList.begin();
	while (itDesc != descriptorsList.end()) {
		if (*itDesc) {
			switch ((*itDesc)->getDescriptorTag()) {
			case 0x40:
				netName = (NetworkName*)(*itDesc);
				cout << "Network name: " << netName->getNetworkName() << endl;
				break;
			case 0xFE:
				sm = (SystemManagement*)(*itDesc);
				cout << "Status: ";
				switch (sm->getBroadcastingFlag()) {
				case SM_BROADCASTING:
					cout << "Broadcasting";
					break;
				case SM_NON_BROADCASTING1:
				case SM_NON_BROADCASTING2:
					cout << "Non-broadcasting";
					break;
				}
				cout << endl;
				cout << "Broadcasting identifier: ";
				switch (sm->getBroadcastingIdentifier()) {
				case ST_UNDEFINED:
					cout << "Undefined";
					break;
				case ST_ISDB_SYSTEM:
					cout << "ISDB System";
					break;
				}
				cout << endl;
				break;
			}
		}
		++itDesc;
	}
	itTrans = transportList.begin();
	while (itTrans != transportList.end()) {
		if (*itTrans) {
			unsigned char letter;
			cout << endl;
			ti = (TransportInformation*) (*itTrans);
			cout << "Transport stream ID: " << ti->transportStreamId << endl;
			cout << "Original network ID: " << ti->originalNetworkId << endl;
			letter = ti->originalNetworkId / 1000;
			cout << "Generation station: ZY";
			switch (letter) {
			case 0:
				cout << "A";
				break;
			case 1:
				cout << "B";
				break;
			case 2:
				cout << "P";
				break;
			case 3:
				cout << "Q";
				break;
			case 4:
				cout << "T";
				break;
			}
			cout << " " << (ti->originalNetworkId % 1000) << endl;
			itDesc = ti->descriptorList.begin();
			while (itDesc != ti->descriptorList.end()) {
				if (*itDesc) {
					switch ((*itDesc)->getDescriptorTag()) {
					case 0x41:
						sl = (ServiceList*)(*itDesc);
						itSl = sl->getServiceList()->begin();
						while (itSl != sl->getServiceList()->end()) {
							cout << "Program: " << itSl->first;
							switch (itSl->second) {
							case DIGITAL_TELEVISION_SERVICE:
								cout << " - Digital television service";
								break;
							case DIGITAL_AUDIO_SERVICE:
								cout << " - Digital audio service";
								break;
							case DATA_BROADCASTING_SERVICE:
								cout << " - Data broadcasting service";
								break;
							case DATA_SERVICE:
								cout << " - Data service";
								break;
							default:
								cout << " - Unknown service";
								break;
							}
							cout << endl;
							++itSl;
						}
						break;
					case 0xCD:
						tsinfo = (TSInformation*)(*itDesc);
						cout << "Virtual channel: " <<
								(tsinfo->getRemoteControlKeyId() & 0xFF);
						cout << endl;
						cout << "TS name: " <<
								tsinfo->getTsName() << endl;
						break;
					case 0xFA:
						tds = (TerrestrialDeliverySystem*)(*itDesc);
						itFl = tds->getFrequencyList()->begin();
						while (itFl != tds->getFrequencyList()->end()) {
							value = *itFl;
							cout << "Broadcast frequency: " <<
									(unsigned short) (value / 7) <<
									" MHz" << endl;
							++itFl;
						}
						cout << "Guard interval: ";
						switch (tds->getGuardInterval()) {
						case GUARD_INTERVAL_1_32:
							cout << "1/32";
							break;
						case GUARD_INTERVAL_1_16:
							cout << "1/16";
							break;
						case GUARD_INTERVAL_1_8:
							cout << "1/8";
							break;
						case GUARD_INTERVAL_1_4:
							cout << "1/4";
							break;
						}
						cout << endl;
						cout << "Transmission mode: " <<
								((tds->getTransmissionMode() + 1) & 0xFF) <<
								endl;
						value = tds->getAreaCode() >> 7;
						cout << "Federation unit: ";
						if (regionList.count(value)) {
							cout << regionList[value];
						} else {
							cout << "Unknown";
						}
						cout << endl;
						break;
					case 0xFB:
						pr = (PartialReception*)(*itDesc);
						cout << "Partial reception programs: ";
						itFl = pr->getServiceIdList()->begin();
						while (itFl != pr->getServiceIdList()->end()) {
							cout << (*itFl);
							++itFl;
							if (itFl != pr->getServiceIdList()->end()) {
								cout << ", ";
							}
						}
						cout << endl;
						break;
					}
				}
				++itDesc;
			}
		}
		++itTrans;
	}

	return (descriptorsList.size() || transportList.size());
}

}
}
}
}


