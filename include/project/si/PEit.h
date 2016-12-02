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

		int encode(int64_t stc, vector<pair<char*,int>*>* list);

};

}
}
}
}


#endif /* PEIT_H_ */
