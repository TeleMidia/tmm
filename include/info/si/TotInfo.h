/*
 * TotInfo.h
 *
 *  Created on: 16/04/2014
 *      Author: Felippe Nagato
 */

#ifndef TOTINFO_H_
#define TOTINFO_H_

#include "si/Tot.h"
#include <iostream>
#include <cstdio>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class TotInfo : public Tot {

	private:

	protected:

	public:
		TotInfo();
		virtual ~TotInfo();

		bool printTable();

		void printDateTime(time_t now, string format,
							bool useUtc, bool printUtc);

};

}
}
}
}


#endif /* TOTINFO_H_ */
