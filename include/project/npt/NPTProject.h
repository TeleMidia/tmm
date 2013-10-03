/*
 * NPTProject.h
 *
 *  Created on: 28/02/2013
 *      Author: Felippe Nagato
 */

#ifndef NPTPROJECT_H_
#define NPTPROJECT_H_

#include "project/ProjectInfo.h"
#include "Dependents.h"
#include "Stc.h"
#include "dsmcc/DSMCCSection.h"
#include "Reference.h"
#include "dsmcc/descriptor/NPTReference.h"
#include "dsmcc/descriptor/NPTEndpoint.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <map>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;
using namespace br::pucrio::telemidia::mpeg2::dsmcc;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class NPTProject : public ProjectInfo, public Subscriber {

	private:
		map<int, vector<Reference*>*>* referenceList;
		map<int, Reference*>* endPointList;
		vector<Reference*>* currRefList;
		int64_t firstReference, priorTime;
		double firstReferenceOffset;
		void init();

	protected:

	public:
		NPTProject();
		NPTProject(int id);
		virtual ~NPTProject();

		int encode(int64_t stc, vector<pair<char*,int>*>* list);

		bool addReference(int id, char cid, int64_t absStart, int64_t absEnd,
						  int64_t nptStart, int64_t nptEnd, short numerator,
						  unsigned short denominator, bool hasNext);
		bool removeReferences(int id);
		bool addEndpoint(int id, int64_t nptStart, int64_t nptEnd);
		bool removeEndPoint(int id);
		vector<Reference*>* getCurrRef(int64_t currStc);
		void setFirstReference(int64_t first);
		double getFirstReferenceOffset();
		void setFirstReferenceOffset(double offset);

};

}
}
}
}


#endif /* NPTPROJECT_H_ */
