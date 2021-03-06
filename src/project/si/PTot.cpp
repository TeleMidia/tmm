/******************************************************************************
TeleM�dia Multiplexer
Copyright (C) 2016 TeleM�dia Lab/PUC-Rio
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
	ProjectInfo();
	init();
}

PTot::PTot(int id) {
	ProjectInfo();
	this->id = id;
	init();
}

PTot::~PTot() {

}

void PTot::init() {
 	offset = 0;
 	useCurrentTime = true;
 	timeBegin = 0;
 	utc = Tot::localTimezone();
	daylightSavingTime = true;
 	projectType = PT_TOT;
 	layer = 0x01; //HIERARCHY_A
 	countryCode.assign("BRA");
 	countryRegionId = 3;//Brazil: RJ - ABNT NBR 15608-3:2008 page 39
}

void PTot::printDateTime(time_t now, string format) {
	short utc = Tot::localTimezone();
	struct tm tstruct;
	char buf[80];

	if (!format.length()) format.assign("%Y-%m-%d %X");
	now += (((int)utc)*60);
	tstruct = *gmtime(&now);
	strftime(buf, sizeof(buf), format.c_str(), &tstruct);
	cout << buf;
	cout << " UTC";
	if (utc >= 0) cout << "+"; else {
		cout << "-";
		utc *= -1;
	}
	cout << (short)(utc/60) << ":";
	printf("%02d", utc%60);
}

char PTot::dayOfWeek(time_t date) {
	return gmtime(&date)->tm_wday;
}

//TODO: Improve this code, please.
time_t PTot::makeUtcDate(const string& dateTime) {
	int dd, mo, yy, hh, mm, ss, uh = 0, um = 0, r;
	struct tm when = {0};
	short utcRef; //in minutes

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

	utcRef = (uh * 60) + um;

	unsigned found = dateTime.find('+');
	if (found==std::string::npos) utcRef *= -1;

	return (mktime(&when) - (utcRef * 60)) + (Tot::localTimezone() * 60); //utc;
}

time_t PTot::makeUtcDate(unsigned short yy, unsigned short mo,
						 unsigned short dd) {
	struct tm when = {0};
	when.tm_mday = dd;
	when.tm_mon = mo - 1;
	when.tm_year = yy - 1900;
	when.tm_hour = 0;
	when.tm_min = 0;
	when.tm_sec = 0;

	return mktime(&when) + (localTimezone() * 60); //utc;
}

time_t PTot::easterSunday(unsigned short wYear) {
	unsigned short wMonth;
	unsigned short wDay;
    unsigned short wCorrection = 0;

    if( wYear < 1700 )      wCorrection = 4;
    else if( wYear < 1800 ) wCorrection = 5;
    else if( wYear < 1900 ) wCorrection = 6;
    else if( wYear < 2100 ) wCorrection = 0;
    else if( wYear < 2200 ) wCorrection = 1;
    else if( wYear < 2300 ) wCorrection = 2;
    else if( wYear < 2500 ) wCorrection = 3;

    wDay = (19 * (wYear % 19) + 24) % 30;
    wDay = 22 + wDay + ((2 * (wYear % 4) + 4 * (wYear % 7) + 6 * wDay + 5 + wCorrection) % 7);

    if( wDay > 31 ) {
        wMonth = 4;
        wDay -= 31;
    } else {
        wMonth = 3;
    }

    return makeUtcDate(wYear, wMonth, wDay);
}

time_t PTot::carnivalSunday(unsigned short year) {
	return easterSunday(year) + (-49*86400);
}

time_t PTot::daylightStart(unsigned short year) {
	time_t out1st = makeUtcDate(year, 10, 1);
	time_t outFirstSunday = out1st + (((7 - dayOfWeek(out1st)) % 7) * 86400);
	return outFirstSunday + (14*86400);
}

time_t PTot::daylightEnd(unsigned short year) {
	time_t feb1st = makeUtcDate(year + 1, 2, 1);
	time_t febFirstSunday = feb1st + (((7 - dayOfWeek(feb1st)) % 7) * 86400);
	return febFirstSunday + (14 * 86400);
}

time_t PTot::nextTimeChange(time_t dateTime, char *dst) {
	struct tm tstruct;
	unsigned short year;
	time_t dBegin, dEnd;
	tstruct = *gmtime(&dateTime);
	year = tstruct.tm_year + 1900;
	dEnd = daylightEnd(year - 1) - 3600;
	if (dEnd - dateTime > 0) {
		year -= 1;
	}
	dBegin = daylightStart(year);
	dEnd = daylightEnd(year);

	if (countryCode == "BRA") { //DECRETO N� 6.558, DE 8 DE SETEMBRO DE 2008.
		//RS, SC, PR, SP, RJ, ES, MG, GO, MT, MS, TO, DF.
		if (dEnd == carnivalSunday(year+1)) {
			dEnd += (7 * 86400);
		}
	}

	if ((dBegin - dateTime) > 0) {
		*dst = 0;
		return dBegin;
	} else {
		*dst = 1;
		return dEnd;
	}
	return true;
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

bool PTot::setTimeBegin(const string& dateTime) {
	int uh = 0, um = 0, r;

	r = sscanf(dateTime.c_str(), "%*d-%*d-%*dT%*d:%*d:%*d-%d:%d", &uh, &um);
	if (r != 2) {
		r = sscanf(dateTime.c_str(), "%*d-%*d-%*dT%*d:%*d:%*d+%d:%d", &uh, &um);
		if (r != 2) return false;
	}

	utc = (uh * 60) + um;

	unsigned found = dateTime.find('+');
	if (found==std::string::npos) utc *= -1;

	timeBegin = makeUtcDate(dateTime);

	return true;
}

time_t PTot::getTimeBegin() {
	return timeBegin;
}

double PTot::updateDateTime(int64_t stc) {
	double elapsedTime = 0.0;

	elapsedTime = Stc::stcToSecond(stc - stcBegin);
	if (useCurrentTime){
		time(&dateTime);
	} else {
		dateTime = timeBegin + (elapsedTime + 0.5f);
	}
	//In the normal way, utcOffset should be equal to zero.
	dateTime += offset + utcOffset;

	return elapsedTime;
}

void PTot::setUtc(short utc) {
	this->utc = utc;
}

short PTot::getUtc() {
	return utc;
}

void PTot::setDaylightSavingTime(bool dst) {
	daylightSavingTime = dst;
}

bool PTot::getDaylightSavingTime() {
	return daylightSavingTime;
}

int PTot::getOffset() {
	return offset;
}

void PTot::setOffset(int os) {
	offset = os;
}

int PTot::getUtcOffset() {
	return utcOffset;
}

//this method causes the UTC time has an offset.
void PTot::setUtcOffset(int uos) {
	utcOffset = uos;
}

void PTot::setCountryCode(const string& country) {
	countryCode.assign(country);
}

void PTot::setCountryRegionId(unsigned char id) {
	countryRegionId = id;
}

int PTot::encode(int64_t stc, vector<pair<char*,int>*>* list) {
	time_t dt;
	double elapsedTime = 0.0;
	char localTimeOffset, dst;

	elapsedTime = updateDateTime(stc);

	releaseAllDescriptors();
	currentNextIndicator = 1;
	lastSectionNumber = 0;
	sectionNumber = 0;
	privateIndicator = 1;
	LocalTimeData* ltd = new LocalTimeData();
	ltd->timeOfChange = nextTimeChange(dateTime, &dst);
	ltd->countryCode = countryCode;
	ltd->countryRegionId = countryRegionId;

	if (countryCode == "BRA") {
		//The weird semantic definition of fields in Brazilian Standard.
		if (((countryRegionId > 0) && (countryRegionId < 3)) ||
			(countryRegionId == 4) || (countryRegionId == 6)) {
			daylightSavingTime = false;
		} else {
			daylightSavingTime = true;
		}
		ltd->localTimeOffsetPolarity = 0;
		if (countryRegionId > 3) ltd->localTimeOffsetPolarity = 1;
		localTimeOffset = 0;
		if ((countryRegionId == 1) || (countryRegionId == 4) ||
			(countryRegionId == 5)) {
			localTimeOffset = 1;
		} else if ((countryRegionId == 6) || (countryRegionId == 7)) {
			localTimeOffset = 2;
		}
	} else {
		//The normal way.
		if (utc < 0) {
			ltd->localTimeOffsetPolarity = 1;
			localTimeOffset = utc * -1;
		} else {
			ltd->localTimeOffsetPolarity = 0;
			localTimeOffset = utc;
		}
	}

	dt = Tot::makeUtcTime(localTimeOffset, 0, 0);
	ltd->localTimeOffset = dt;
	dt = 0;
	if (daylightSavingTime) {
		if (!dst) {
			dt = Tot::makeUtcTime(1, 0, 0);
		} else {
			dateTime += 3600;
		}
	}
	ltd->nextTimeOffset = dt;
	LocalTimeOffset* lto = new LocalTimeOffset();
	lto->addLocalTimeData(ltd);
	addDescriptor(lto);

	int length = updateStream();
	pair<char*,int>* streamData = new pair<char*,int>;
	streamData->first = stream;
	streamData->second = length;

	list->push_back(streamData);

	cout << "tot = ";
	printDateTime(dateTime - utcOffset, "");
	cout << " ~ elapsed time = " << (int64_t) elapsedTime << endl;

	return 0;
}

}
}
}
}


