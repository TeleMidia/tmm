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
