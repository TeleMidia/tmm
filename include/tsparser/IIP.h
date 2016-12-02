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
 * IIP.h
 *
 *  Created on: 24/09/2013
 *      Author: Felippe Nagato
 */

#ifndef IIP_H_
#define IIP_H_

#include "MCCI.h"
#include <iostream>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {
namespace isdbt {

class IIP {
	private:

	protected:
		char* stream;
		unsigned short currentSize;

		unsigned short IIPPacketPointer;
		MCCI* mcci;
		unsigned char IIPBranchNumber;
		unsigned char lastIIPBranchNumber;
		unsigned char nsiLength;
		//NSI* nsi //TODO: Network Synchronization Information

	public:
		IIP();
		virtual ~IIP();

		int addData(char* data, unsigned short length);

		void setIIPPacketPointer(unsigned short pointer);
		void setMcci(MCCI* mcci);
		void setIIPBranchNumber(unsigned char num);
		void setlastIIPBranchNumber(unsigned char last);

		unsigned short getIIPPacketPointer();
		MCCI* getMcci();
		unsigned char getIIPBranchNumber();
		unsigned char getlastIIPBranchNumber();
		unsigned char getNsiLength();

		int updateStream();
		int getStream(char** dataStream);

};

}
}
}
}
}


#endif /* IIP_H_ */
