/*
 * TotInfo.cpp
 *
 *  Created on: 16/04/2014
 *      Author: Felippe Nagato
 */

#include "info/si/TotInfo.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

TotInfo::TotInfo() {

}

TotInfo::~TotInfo() {

}

void TotInfo::printDateTime(time_t now, string format,
							bool useUtc, bool printUtc) {
	short utc = Tot::localTimezone();
	struct tm tstruct;
	char buf[80];

	if (!format.length()) format.assign("%Y-%m-%d %X");
	if (useUtc) now += (((int)utc)*60);
	tstruct = *gmtime(&now);
	strftime(buf, sizeof(buf), format.c_str(), &tstruct);
	cout << buf;
	if (printUtc) {
		cout << " UTC";
		if (utc >= 0) cout << "+"; else {
			cout << "-";
			utc *= -1;
		}
		cout << (short)(utc/60) << ":";
		printf("%02d", utc%60);
	}
}

bool TotInfo::printTable() {
	vector<MpegDescriptor*>::iterator it;
	LocalTimeOffset* lto;
	vector<LocalTimeData*>::iterator itLt;

	cout << "TOT - Time Offset Table" << endl <<
			"-----------------------" << endl << endl;

	cout << "Date/Time: ";
	printDateTime(dateTime, "", true, true);
	cout << endl;

	it = descriptorList.begin();
	while (it != descriptorList.end()) {
		switch ((*it)->getDescriptorTag()) {
		case 0x58:
			lto = (LocalTimeOffset*)(*it);
			itLt = lto->getLocalTimeDataList()->begin();
			while (itLt != lto->getLocalTimeDataList()->end()) {
				cout << "Local time offset: ";
				printDateTime((*itLt)->localTimeOffset, "%X", false, false);
				cout << endl;
				cout << "Time of change: ";
				printDateTime((*itLt)->timeOfChange, "", false, false);
				cout << endl;
				cout << "Next time offset: ";
				printDateTime((*itLt)->nextTimeOffset, "%X", false, false);
				cout << endl;
				cout << "Local time offset polarity: " <<
						((*itLt)->localTimeOffsetPolarity & 0xFF) << endl;
				cout << "Country: " << (*itLt)->countryCode << endl;
				cout << "Region ID: " << ((*itLt)->countryRegionId & 0xFF) <<
						endl;
				++itLt;
			}
			break;
		}
		++it;
	}

	return true;
}

}
}
}
}


