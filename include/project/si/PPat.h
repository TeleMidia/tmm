/*
 * PPat.h
 *
 *  Created on: 04/09/2013
 *      Author: Felippe Nagato
 */

#ifndef PPAT_H_
#define PPAT_H_

#include "project/ProjectInfo.h"
#include "si/Pat.h"
#include <iostream>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class PPat : public ProjectInfo, public Pat {

	private:

	protected:

	public:
		PPat(){projectType = PT_PAT;};
		virtual ~PPat(){};

};

}
}
}
}


#endif /* PPAT_H_ */
