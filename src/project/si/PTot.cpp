/*
 * PTot.cpp
 *
 *  Created on: 03/05/2013
 *      Author: Felippe Nagato
 */

#include "project/si/PTot.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

PTot::PTot() {
	offset = 0;
	useCurrentTime = true;
	timeBegin = 0;
	utc = Tot::localTimezone();
	daylightSavingTime = false;
	projectType = PT_TOT;
	countryCode.assign("BRA");
}

PTot::PTot(int id) {
	this->id = id;
	offset = 0;
	useCurrentTime = true;
	timeBegin = 0;
	utc = Tot::localTimezone();
	daylightSavingTime = false;
	projectType = PT_TOT;
	countryCode.assign("BRA");
}

PTot::~PTot() {

}

void PTot::printDateTime(time_t now, string format) {
	struct tm tstruct;
	char buf[80];
	if (!format.length()) format.assign("%Y-%m-%d %X");
	tstruct = *gmtime(&now);
	strftime(buf, sizeof(buf), format.c_str(), &tstruct);
	cout << buf;
}

void PTot::setStcBegin(int64_t stc) {
	stcBegin = stc;
}

void PTot::setUseCurrentTime(bool ct) {
	useCurrentTime = ct;
}

bool PTot::getUseCurrentTime() {
	return useCurrentTime;
}

//TODO: Improve this code, please.
bool PTot::setTimeBegin(string dateTime) {
	int dd, mo, yy, hh, mm, ss, uh = 0, um = 0, r;
	struct tm when = {0};

	r = sscanf(dateTime.c_str(), "%d-%d-%dT%d:%d:%d-%d:%d",
			&yy, &mo, &dd, &hh, &mm, &ss, &uh, &um);
	if (r != 8) {
		r = sscanf(dateTime.c_str(), "%d-%d-%dT%d:%d:%d+%d:%d",
				&yy, &mo, &dd, &hh, &mm, &ss, &uh, &um);
		if (r != 8) return false;
	}

	when.tm_mday = dd;
	when.tm_mon = mo - 1;
	when.tm_year = yy - 1900;
	when.tm_hour = hh;
	when.tm_min = mm;
	when.tm_sec = ss;

	utc = uh;

	unsigned found = dateTime.find('+');
	if (found==std::string::npos) utc *= -1;

	timeBegin = mktime(&when) + (Tot::localTimezone() * 3600); //utc;

	return true;
}

time_t PTot::getTimeBegin() {
	return timeBegin;
}

void PTot::setUtc(char utc) {
	this->utc = utc;
}

char PTot::getUtc() {
	return utc;
}

void PTot::setDaylightSavingTime(bool dst) {
	daylightSavingTime = dst;
}

bool PTot::getDaylightSavingTime() {
	return daylightSavingTime;
}

void PTot::setOffset(int os) {
	offset = os;
}

void PTot::setCountryCode(string country) {
	countryCode.assign(country);
}

int PTot::encodeSections(int64_t stc, vector<PrivateSection*>* list) {
	time_t dateTime;
	double elapsedTime = 0.0;
	char localTimeOffset;

	elapsedTime = Stc::stcToSecond(stc - stcBegin);
	if (useCurrentTime){
		time(&dateTime);
	} else {
		dateTime = timeBegin + (elapsedTime + 0.5f);
	}
	dateTime += offset;
	cout << "tot = ";
	printDateTime(dateTime, "");
	cout << " ~ elapsed time = " << (int64_t) elapsedTime << endl;

	Tot* tot = new Tot();
	tot->setCurrentNextIndicator(1);
	tot->setLastSectionNumber(0);
	tot->setSectionNumber(0);
	tot->setPrivateIndicator(1);
	tot->setDateTime(dateTime);
	LocalTimeData* ltd = new LocalTimeData();
	ltd->timeOfChange = 0; //hardcoded
	ltd->countryCode = countryCode;
	ltd->countryRegionId = 28; //hardcoded - is it correct?
	if (utc < 0) {
		ltd->localTimeOffsetPolarity = 1;
		localTimeOffset = utc * -1;
	} else {
		ltd->localTimeOffsetPolarity = 0;
		localTimeOffset = utc;
	}
	dateTime = Tot::makeUtcTime(localTimeOffset, 0, 0);
	ltd->localTimeOffset = dateTime;
	dateTime = Tot::makeUtcTime(1, 0, 0);
	ltd->nextTimeOffset = dateTime; //hardcoded
	LocalTimeOffset* lto = new LocalTimeOffset();
	lto->addLocalTimeData(ltd);
	tot->addDescriptor(lto);
	tot->updateStream();

	list->push_back(tot);

	return 0;
}

}
}
}
}


