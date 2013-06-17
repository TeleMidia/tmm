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

char PTot::dayOfWeek(time_t date) {
	return gmtime(&date)->tm_wday;
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

	return mktime(&when) + (localTimezone() * 3600); //utc;
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
	dBegin = daylightStart(year);
	dEnd = daylightEnd(year);

	if (countryCode == "BRA") { //DECRETO Nº 6.558, DE 8 DE SETEMBRO DE 2008.
		//RS, SC, PR, SP, RJ, ES, MG, GO, MT, MS, TO, DF.
		if (dEnd == carnivalSunday(year+1)) {
			dEnd += (7 * 86400);
		}
	}

	if ((dBegin - dateTime) >= 0) {
		*dst = 1;
		return dBegin;
	} else {
		*dst = 0;
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
	char localTimeOffset, dst;

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
	ltd->timeOfChange = nextTimeChange(dateTime, &dst) + 3600;
	ltd->countryCode = countryCode;
	ltd->countryRegionId = 0; //hardcoded - is it correct?
	if (utc < 0) {
		ltd->localTimeOffsetPolarity = 1;
		localTimeOffset = utc * -1;
	} else {
		ltd->localTimeOffsetPolarity = 0;
		localTimeOffset = utc;
	}
	dateTime = Tot::makeUtcTime(localTimeOffset, 0, 0);
	ltd->localTimeOffset = dateTime;
	dateTime = Tot::makeUtcTime(dst, 0, 0);
	ltd->nextTimeOffset = dateTime;
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


