/*
 * IIPInfo.h
 *
 *  Created on: 25/04/2014
 *      Author: Felippe Nagato
 */

#ifndef IIPINFO_H_
#define IIPINFO_H_

#include "tsparser/IIP.h"
#include <iostream>
#include <cstdio>

using namespace std;
using namespace br::pucrio::telemidia::tool::isdbt;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class IIPInfo : public IIP {

	private:

	protected:

	public:
		IIPInfo();
		virtual ~IIPInfo();

		bool printTable();

};

}
}
}
}

#endif /* IIPINFO_H_ */
