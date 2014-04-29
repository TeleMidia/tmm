/*
 * PatInfo.h
 *
 *  Created on: 16/04/2014
 *      Author: Felippe Nagato
 */

#ifndef PATINFO_H_
#define PATINFO_H_

#include "si/Pat.h"
#include <iostream>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class PatInfo : public Pat {

	private:

	protected:

	public:
		PatInfo();
		virtual ~PatInfo();

		bool printTable();

};

}
}
}
}


#endif /* PATINFO_H_ */
