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
 * Timeline.h
 *
 *  Created on: 25/03/2013
 *      Author: Felippe Nagato
 */

#ifndef TIMELINE_H_
#define TIMELINE_H_

#include "Stc.h"
#include "project/si/PMTView.h"
#include "project/ProjectInfo.h"
#include <iostream>
#include <map>
#include <vector>
#include <cstdlib>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

struct pmtViewInfo {
	 int priorPmtId;
	 int64_t duration;
	 PMTView* pv;
};

class Timeline : public ProjectInfo {

	private:

	protected:
		bool isLoop;
		vector<pmtViewInfo*>* previousTimeline;
		vector<pmtViewInfo*>* currentTimeline;
		unsigned int currentTimelineBegin;
		map<int64_t, vector<pmtViewInfo*>*>* timelineList;
		void removeTimelineList();

	public:
		Timeline();
		virtual ~Timeline();

		void addTimeline(int64_t time, int64_t duration, PMTView* pmtView,
						 int priorId);
		bool removeOldTimelines(int64_t relStc);
		vector<pmtViewInfo*>* currTimeline(int64_t relStc, int* condRet);
		vector<pmtViewInfo*>* getCurrentTimeline();
		vector<pmtViewInfo*>* getPreviousTimeline();
		vector<pmtViewInfo*>* getLastTimeline();
		unsigned int getCurrTimelineBegin();
		void setIsLoop(bool loop);
		bool getIsLoop();

};

}
}
}
}


#endif /* TIMELINE_H_ */
