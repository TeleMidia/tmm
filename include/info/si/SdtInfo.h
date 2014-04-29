/*
 * SdtInfo.h
 *
 *  Created on: 16/04/2014
 *      Author: Felippe Nagato
 */

#ifndef SDTINFO_H_
#define SDTINFO_H_

#include "si/Sdt.h"
#include <iostream>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class SdtInfo : public Sdt {

	private:

	protected:

	public:
	SdtInfo();
		virtual ~SdtInfo();

		bool printTable();

};

}
}
}
}


#endif /* SDTINFO_H_ */
