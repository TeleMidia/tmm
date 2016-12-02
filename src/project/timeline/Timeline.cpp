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

bool Timeline::removeOldTimelines(int64_t relStc) {
	map<int64_t, vector<pmtViewInfo*>*>::iterator it;
	vector<pmtViewInfo*>::iterator itV;

	if (timelineList->size() < 2) return false;

	it = timelineList->begin();
	while (it != timelineList->end()) {
		if ((it == timelineList->begin()) &&
			((Stc::stcToSecond(relStc)*1000) >= it->first)) {
			if (it->second) {
				itV = it->second->begin();
				while (itV != it->second->end()) {
					delete (*itV);
					++itV;
				}
				delete it->second;
			}
			timelineList->erase(it);
			if (timelineList->size() < 2) return true;
			it = timelineList->begin();
			continue;
		}
		++it;
	}

	return true;
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
				uint64_t startTime = 0;
				if (!isLoop) {
					if (condRet) *condRet = 3; //End of the timeline.
					return NULL;
				} else {
					//TODO: Add a new project from here or

					//use current overall project. Updating each timeline...
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


