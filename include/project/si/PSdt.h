/*
 * PSdt.h
 *
 *  Created on: 18/06/2013
 *      Author: Felippe Nagato
 */

#ifndef PSDT_H_
#define PSDT_H_

#include "project/ProjectInfo.h"
#include "si/Sdt.h"
#include <iostream>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class PSdt : public ProjectInfo, public Sdt {

	private:

	protected:

	public:
		PSdt();
		virtual ~PSdt();

};

}
}
}
}


#endif /* PSDT_H_ */
