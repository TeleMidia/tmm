/******************************************************************************
TeleMídia Multiplexer
Copyright (C) 2016 TeleMídia Lab/PUC-Rio
https://github.com/TeleMidia/tmm-mpeg2ts/graphs/contributors

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU Affero General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version. This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
for more details. You should have received a copy of the GNU Affero General
Public License along with this program. If not, see http://www.gnu.org/licenses/.

*******************************************************************************/

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
