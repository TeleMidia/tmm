/*
 * NitInfo.h
 *
 *  Created on: 15/04/2014
 *      Author: Felippe Nagato
 */

#ifndef NITINFO_H_
#define NITINFO_H_

#include "si/Nit.h"
#include "si/Sdt.h"
#include <iostream>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class NitInfo : public Nit {

	private:

	protected:
		map<unsigned char, string> regionList;
	public:
		NitInfo();
		virtual ~NitInfo();

		bool printTable();

};

}
}
}
}


#endif /* NITINFO_H_ */
