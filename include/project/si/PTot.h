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
 * PTot.h
 *
 *  Created on: 03/05/2013
 *      Author: Felippe Nagato
 */

#ifndef PTOT_H_
#define PTOT_H_

#include "project/ProjectInfo.h"
#include "si/Tot.h"
#include "Dependents.h"
#include "Stc.h"

#include <iostream>
#include <cstdio>
#include <string>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class PTot : public ProjectInfo, public Tot, public Subscriber {

	private:
		void init();

	protected:
		int64_t stcBegin; // for reference purpose
		int offset; //Offset used to correct the time discrepancy caused by stcBegin.
		int utcOffset;

		bool useCurrentTime; // if TOT should use system time
		time_t timeBegin; // date and time when the muxer starts (from project)
		short utc;
		bool daylightSavingTime;
		string countryCode;
		unsigned char countryRegionId;

	public:
		PTot();
		PTot(int id);
		virtual ~PTot();

		static void printDateTime(time_t now, string format);

		void setStcBegin(int64_t stc);
		void setUseCurrentTime(bool ct);
		bool getUseCurrentTime();
		bool setTimeBegin(const string& dateTime);
		time_t getTimeBegin();
		double updateDateTime(int64_t stc);
		void setUtc(short utc);
		short getUtc();
		void setDaylightSavingTime(bool dst);
		bool getDaylightSavingTime();
		int getOffset();
		void setOffset(int os);
		int getUtcOffset();
		void setUtcOffset(int uos);
		void setCountryCode(const string& country);
		void setCountryRegionId(unsigned char id);
		time_t nextTimeChange(time_t dateTime, char *dst);

		static char dayOfWeek(time_t date);
		static time_t makeUtcDate(const string& dateTime);
		static time_t makeUtcDate(unsigned short yy, unsigned short mo,
								  unsigned short dd);
		static time_t easterSunday(unsigned short wYear);
		static time_t carnivalSunday(unsigned short year);
		static time_t daylightStart(unsigned short year);
		static time_t daylightEnd(unsigned short year);

		int encode(int64_t stc, vector<pair<char*,int>*>* list);

};

}
}
}
}


#endif /* PTOT_H_ */
