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
 * PStreamEvent.h
 *
 *  Created on: 03/02/2014
 *      Author: Felippe Nagato
 */

#ifndef PSTREAMEVENT_H_
#define PSTREAMEVENT_H_

#include "project/ProjectInfo.h"
#include "Dependents.h"
#include "Stc.h"
#include "dsmcc/DSMCCSection.h"
#include "dsmcc/descriptor/StreamEvent.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <map>
#include <stdlib.h>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;
using namespace br::pucrio::telemidia::mpeg2::dsmcc;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class PStreamEvent : public ProjectInfo, public Subscriber {

	private:
		unsigned int sampleCount;

	protected:
		int64_t firstReference;
		double firstReferenceOffset;
		unsigned int period, sampleLimit;
		vector<StreamEvent*> streamEventList;
		ProjectInfo* carouselProj;
		string entryPoint;
		string baseId;
		string documentId;
		bool processed;

	public:
		PStreamEvent();
		virtual ~PStreamEvent();

		int encode(int64_t stc, vector<pair<char*,int>*>* list);

		void setFirstReference(int64_t first);
		void setPeriod(unsigned int p);
		unsigned int getPeriod();
		void setSampleLimit(unsigned int sl);
		unsigned int getSampleLimit();
		void addStreamEvent(StreamEvent* se);
		double getFirstReferenceOffset();
		void setFirstReferenceOffset(double offset);
		void setCarouselProj(ProjectInfo* proj);
		ProjectInfo* getCarouselProj();
		void setEntryPoint(const string& ep);
		string getEntryPoint();
		void setBaseId(unsigned short bi);
		void setBaseId(const string& bi);
		string getBaseId();
		void setDocumentId(const string& di);
		string getDocumentId();
		vector<StreamEvent*>* getStreamEventList();
		void resetSampleCount();
		void setProcessed(bool processed);
		bool getProcessed();

};

}
}
}
}


#endif /* PSTREAMEVENT_H_ */
