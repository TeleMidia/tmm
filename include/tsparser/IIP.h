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

		unsigned short IIPPacketPointer;
		MCCI* mcci;
		unsigned char IIPBranchNumber;
		unsigned char lastIIPBranchNumber;
		unsigned char nsiLength;
		//NSI* nsi //TODO: Network Synchronization Information

	public:
		IIP();
		virtual ~IIP();

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
