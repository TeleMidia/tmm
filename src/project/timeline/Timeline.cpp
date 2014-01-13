/*
 * Timeline.cpp
 *
 *  Created on: 25/03/2013
 *      Author: Felippe Nagato
 */

#include "project/timeline/Timeline.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

Timeline::Timeline() {
	timelineList = new map<int64_t, vector<pmtViewInfo*>*>;
	previousTimeline = NULL;
	currentTimeline = NULL;
	currentTimelineBegin = 0;
	projectType = PT_TIMELINE;
	isLoop = false;
}

Timeline::~Timeline() {
	removeTimelineList();
	delete timelineList;
}

void Timeline::removeTimelineList() {
	map<int64_t, vector<pmtViewInfo*>*>::iterator it;
	vector<pmtViewInfo*>::iterator itV;

	it = timelineList->begin();
	while (it != timelineList->end()) {
		if (it->second) {
			itV = it->second->begin();
			while (itV != it->second->end()) {
				delete (*itV);
				++itV;
			}
			delete it->second;
		}
		++it;
	}
	timelineList->clear();
}

void Timeline::addTimeline(int64_t time, int64_t duration, PMTView* pmtView,
						   int priorId) {
	map<int64_t, vector<pmtViewInfo*>*>::iterator it;
	vector<pmtViewInfo*>* pmtViewList = NULL;
	pmtViewInfo* pvi;

	it = timelineList->find(time);
	if (it == timelineList->end()) {
		if (pmtView) {
			pmtViewList = new vector<pmtViewInfo*>;
		}
		(*timelineList)[time] = pmtViewList;
	} else {
		pmtViewList = it->second;
	}

	if (pmtView) {
		pvi = new pmtViewInfo;
		pvi->priorPmtId = priorId;
		pvi->pv = pmtView;
		pvi->duration = duration;
		pmtViewList->push_back(pvi);
	}

}

vector<pmtViewInfo*>* Timeline::currTimeline(int64_t relStc, int* condRet) {
	map<int64_t, vector<pmtViewInfo*>*>::iterator itList;
	map<int64_t, vector<pmtViewInfo*>*>::reverse_iterator it;
	vector<pmtViewInfo*>::iterator itV;

	if (condRet) *condRet = 0;

	it = timelineList->rbegin();
	while (it != timelineList->rend()) {
		if (Stc::stcToSecond(relStc)*1000 >= it->first) {
			if (!it->second) {
				map<int64_t, vector<pmtViewInfo*>*>* tempList;
				int64_t startTime = 0;
				if (!isLoop) {
					if (condRet) *condRet = 3; //End
					return NULL;
				} else {
					//TODO: Add a new project from here or

					//use current project.
					startTime = it->first;
					tempList = new map<int64_t, vector<pmtViewInfo*>*>;

					itList = timelineList->begin();
					while (itList != timelineList->end()) {
						(*tempList)[startTime] = itList->second;
						if (itList->second) {
							itV = itList->second->begin();
							if (itV != itList->second->end()) {
								startTime += (*itV)->duration;
							}
						}
						++itList;
					}
					delete timelineList;
					timelineList = tempList;
					it = timelineList->rbegin();
					if (condRet) *condRet = 2; //Loop
					continue;
				}
			}
			if (currentTimeline != it->second) {
				previousTimeline = currentTimeline;
				currentTimeline = it->second;
				currentTimelineBegin = it->first;
				if (condRet) {
					if (*condRet != 2) {
						*condRet = 1; //timeline changed
					}
				}
			} else {
				if (condRet) {
					if (*condRet != 2) {
						*condRet = 0; //timeline not modified
					}
				}
			}
			return it->second;
		}
		++it;
	}
	if (condRet) *condRet = -1; //Error
	return NULL;
}

vector<pmtViewInfo*>* Timeline::getCurrentTimeline() {
	return currentTimeline;
}

vector<pmtViewInfo*>* Timeline::getLastTimeline() {
	map<int64_t, vector<pmtViewInfo*>*>::reverse_iterator it;

	it = timelineList->rbegin();
	while (it != timelineList->rend()) {
		if (it->second) return it->second;
		++it;
	}
	return NULL;
}

vector<pmtViewInfo*>* Timeline::getPreviousTimeline() {
	return previousTimeline;
}

unsigned int Timeline::getCurrTimelineBegin() {
	return currentTimelineBegin;
}

void Timeline::setIsLoop(bool loop) {
	isLoop = loop;
}

bool Timeline::getIsLoop() {
	return isLoop;
}

}
}
}
}


