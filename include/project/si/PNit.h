/*
 * PNit.h
 *
 *  Created on: 22/06/2013
 *      Author: Felippe Nagato
 */

#ifndef PNIT_H_
#define PNIT_H_

#include "project/ProjectInfo.h"
#include "si/Nit.h"
#include <iostream>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class PNit : public ProjectInfo, public Nit {

	private:

	protected:

	public:
		PNit();
		virtual ~PNit();

};

}
}
}
}


#endif /* PNIT_H_ */
