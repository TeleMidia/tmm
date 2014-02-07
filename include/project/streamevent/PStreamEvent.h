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
		void setEntryPoint(string ep);
		string getEntryPoint();
		void setBaseId(unsigned short bi);
		void setBaseId(string bi);
		string getBaseId();
		void setDocumentId(string di);
		string getDocumentId();
		vector<StreamEvent*>* getStreamEventList();
		void resetSampleCount();

};

}
}
}
}


#endif /* PSTREAMEVENT_H_ */
