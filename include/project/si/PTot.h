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

	protected:
		int64_t stcBegin; // for reference purpose
		int offset;

		bool useCurrentTime; // if TOT should use system time
		time_t timeBegin; // date and time when the muxer starts (from project)
		char utc;
		bool daylightSavingTime;
		string countryCode;
	public:
		PTot();
		PTot(int id);
		virtual ~PTot();

		static void printDateTime(time_t now, string format);

		void setStcBegin(int64_t stc);
		void setUseCurrentTime(bool ct);
		bool getUseCurrentTime();
		bool setTimeBegin(string dateTime);
		time_t getTimeBegin();
		void setUtc(char utc);
		char getUtc();
		void setDaylightSavingTime(bool dst);
		bool getDaylightSavingTime();
		void setOffset(int os);
		void setCountryCode(string country);
		time_t nextTimeChange(time_t dateTime, char *dst);

		static char dayOfWeek(time_t date);
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
