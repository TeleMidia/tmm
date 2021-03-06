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
 * PEit.cpp
 *
 *  Created on: 02/08/2013
 *      Author: Felippe Nagato
 */

#include "project/si/PEit.h"
#include <cstdio>

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

PEit::PEit() {
	ProjectInfo();
	init();
}

PEit::PEit(int id) {
	ProjectInfo();
	init();
	this->id = id;
}

void PEit::init() {
	projectType = PT_EIT_PF;
	lastEventId = -1;
	offsetAdjusted = false;
	layer = 0x01; //HIERARCHY_A
}

PEit::~PEit() {

}

//Jury-rigging the UTC time.
void PEit::adjustUtcOffset(int uos) {
	map<unsigned short, EventInfo*>::iterator itEvent;

	if (offsetAdjusted) return;
	itEvent = eventList.begin();
	while (itEvent != eventList.end()) {
		if (itEvent->second) {
			itEvent->second->startTime += uos;
		}
		++itEvent;
	}
	offsetAdjusted = true;
}

void PEit::setStcBegin(int64_t stc) {
	stcBegin = stc;
}

void PEit::setTimeBegin(time_t dateTime) {
	timeBegin = dateTime;
}

time_t PEit::getTimeBegin() {
	return timeBegin;
}

int PEit::encode(int64_t stc, vector<pair<char*,int>*>* list) {
	map<unsigned short, EventInfo*>::iterator it;
	map<unsigned short, EventInfo*>::reverse_iterator rit;
	unsigned char sn;
	time_t dateTime;
	double elapsedTime;
	int eventId = -1;

	elapsedTime = Stc::stcToSecond(stc - stcBegin);
	dateTime = timeBegin + (elapsedTime + 0.5f);

	//Present event   - Located in section 0
	//Following event - Located in section 1

	for (sn = 0; sn < 2; sn++) {
		Eit* eit = new Eit();
		if (sn == 0) {
			it = eventList.begin();
			while (it != eventList.end()) {
				if (it->second) {
					it->second->runningStatus = RS_NOT_RUNNING;
					if ((dateTime >= it->second->startTime) &&
						(dateTime < (it->second->startTime + it->second->duration))) {
						//event occurring
						it->second->runningStatus = RS_RUNNING;
						eit->addEventInfo(it->second);
						eventId = it->first;
						if (eventId != lastEventId) {
							versionNumber++;
							lastEventId = eventId;
							/*tm *t=localtime(&dateTime);
							cout << tableIdExtension << ": " << eventId << ", " <<
								Stc::stcToSecond(stc - stcBegin) << ", ";
								printf("%04d-%02d-%02d %02d:%02d:%02d\n",
								t->tm_year+1900,t->tm_mon+1,t->tm_mday,
								t->tm_hour,t->tm_min,t->tm_sec);*/
						}
						break;
					}
				}
				++it;
			}
			if ((eventId == -1) && (lastEventId != -1)) {
				//outside EIT timeline
				rit = eventList.rbegin();
				if (rit != eventList.rend()) {
					if (rit->first != lastEventId) {
						//replicating last event.
						it = eventList.find(lastEventId);
						if (it != eventList.end()) {
							it->second->runningStatus = RS_RUNNING;
							eit->addEventInfo(it->second);
							eventId = it->first;
						}
					} else {
						//table with nothing inside (EIT timeline unavailable).
						lastEventId = -1;
						versionNumber++;
					}
				}
			}
		} else {
			if (eventId >= 0) {
				it = eventList.find(eventId);
				it++;
				if (it != eventList.end()) {
					//event not running yet
					eit->addEventInfo(it->second);
				}
			}
		}
		eit->setSectionNumber(sn);
		eit->setLastSectionNumber(1);
		eit->setPrivateIndicator(1);
		eit->setTableIdExtension(tableIdExtension);
		eit->setCurrentNextIndicator(1);
		eit->setVersionNumber(versionNumber);
		eit->setSegmentLastSectionNumber(1);
		eit->setLastTableId(0x4E);
		eit->setTransportStreamId(transportStreamId);
		eit->setOriginalNetworkId(originalNetworkId);
		eit->setDestroyEvents(false);
		eit->updateStream();

		char* tempStream;
		pair<char*,int>* myp = new pair<char*,int>;
		int length = eit->getStream(&tempStream);
		myp->first = new char[length];
		memcpy(myp->first, tempStream, length);
		myp->second = length;
		list->push_back(myp);

		delete eit;
	}

	return 0;
}

}
}
}
}
