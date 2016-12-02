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
