/*
 * PmtInfo.h
 *
 *  Created on: 16/04/2014
 *      Author: Felippe Nagato
 */

#ifndef PMTINFO_H_
#define PMTINFO_H_

#include "si/Pmt.h"
#include <iostream>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class PmtInfo : public Pmt {

	private:

	protected:
		unsigned short pid;

	public:
		PmtInfo();
		virtual ~PmtInfo();

		void setPid(unsigned short pid);

		bool printTable();

};

}
}
}
}


#endif /* PMTINFO_H_ */
