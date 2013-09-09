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
		int offset;
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

		static void printDateTime(time_t now, string format, bool local);

		void setStcBegin(int64_t stc);
		void setUseCurrentTime(bool ct);
		bool getUseCurrentTime();
		bool setTimeBegin(string dateTime);
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
		void setCountryCode(string country);
		void setCountryRegionId(unsigned char id);
		time_t nextTimeChange(time_t dateTime, char *dst);

		static char dayOfWeek(time_t date);
		static time_t makeUtcDate(string dateTime);
		static time_t makeUtcDate(unsigned short yy, unsigned short mo,
								  unsigned short dd);
		static time_t easterSunday(unsigned short wYear);
		static time_t carnivalSunday(unsigned short year);
		static time_t daylightStart(unsigned short year);
		static time_t daylightEnd(unsigned short year);

		int encodeSections(int64_t stc, vector<PrivateSection*>* list);

};

}
}
}
}


#endif /* PTOT_H_ */
