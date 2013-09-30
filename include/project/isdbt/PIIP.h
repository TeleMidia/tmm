/*
 * PIIP.h
 *
 *  Created on: 26/09/2013
 *      Author: Felippe Nagato
 */

#ifndef PIIP_H_
#define PIIP_H_

#include "project/ProjectInfo.h"
#include "tsparser/IIP.h"
#include "Dependents.h"

#include <iostream>
#include <cstdio>
#include <string>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;
using namespace br::pucrio::telemidia::tool::isdbt;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class PIIP : public ProjectInfo, public IIP, public Subscriber {

	private:
		void init();

	protected:
		unsigned char currentSynch;

	public:
		PIIP();
		PIIP(int id);
		virtual ~PIIP();

		int encode(int64_t stc, vector<pair<char*,int>*>* list);

};

}
}
}
}

#endif /* PIIP_H_ */
