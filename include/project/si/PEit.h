/*
 * PEit.h
 *
 *  Created on: 01/08/2013
 *      Author: Felippe Nagato
 */

#ifndef PEIT_H_
#define PEIT_H_

#include "project/ProjectInfo.h"
#include "si/Eit.h"
#include "Dependents.h"
#include "Stc.h"
#include <iostream>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class PEit : public ProjectInfo, public Eit, public Subscriber {

	private:
		void init();

	protected:
		unsigned int turnNumber;
		int64_t stcBegin; // for reference purpose
		time_t timeBegin; // date and time when the muxer starts (from project)
		int lastEventId;
		bool offsetAdjusted;

		//TODO: Allow events working with the system time.

	public:
		PEit();
		PEit(int id);
		virtual ~PEit();

		void adjustUtcOffset(int uos);
		void setStcBegin(int64_t stc);
		void setTimeBegin(time_t dateTime);
		time_t getTimeBegin();

		int encodeSections(int64_t stc, vector<PrivateSection*>* list);

};

}
}
}
}


#endif /* PEIT_H_ */
