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
