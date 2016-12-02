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
 * ISDBTInformation.h
 *
 *  Created on: 16/09/2013
 *      Author: Felippe Nagato
 */

#ifndef ISDBTINFORMATION_H_
#define ISDBTINFORMATION_H_

#include <iostream>
#include <cstring>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {
namespace isdbt {

class ISDBTInformation {
	#define BS_DIGITAL 0x00
	#define TERRESTRIAL_DIGITAL_TV 0x02
	#define TERRESTRIAL_DIGITAL_AUDIO 0x03
	#define NULL_TSP 0x00
	#define HIERARCHY_A 0x01
	#define HIERARCHY_B 0x02
	#define HIERARCHY_C 0x03
	#define ACDATA_NO_HIERARCHY 0x04
	#define IIP_NO_HIERARCHY 0x08

	private:

	protected:
		char* stream;
		unsigned char TMCCIdentifier;
		bool bufferResetControlFlag;
		bool switchOnControlFlagForEmergencyBroadcasting;
		bool initializationTimingHeadPacketFlag;
		bool frameHeadPacketFlag;
		bool frameIndicator;
		unsigned char layerIndicator;
		unsigned char countDownIndex;
		bool ACDataInvalidFlag;
		unsigned char ACDataEffectiveBytes;
		unsigned short TSPCounter;
		char* ACData;
		unsigned short ACDataLength;

		void init();

	public:
		ISDBTInformation();
		ISDBTInformation(char* data);
		virtual ~ISDBTInformation();

		void setTMCCIdentifier(unsigned char id);
		unsigned char getTMCCIdentifier();
		void setBufferResetControlFlag(bool flag);
		bool getBufferResetControlFlag();
		void setSwitchOnControlFlagForEmergencyBroadcasting(bool flag);
		bool getSwitchOnControlFlagForEmergencyBroadcasting();
		void setInitializationTimingHeadPacketFlag(bool flag);
		bool getInitializationTimingHeadPacketFlag();
		void setFrameHeadPacketFlag(bool flag);
		bool getFrameHeadPacketFlag();
		void setFrameIndicator(bool indicator);
		bool getFrameIndicator();
		void setLayerIndicator(unsigned char layer);
		unsigned char getLayerIndicator();
		void setCountDownIndex(unsigned char cindex);
		unsigned char getCountDownIndex();
		void setACDataInvalidFlag(bool flag);
		bool getACDataInvalidFlag();
		void setACDataEffectiveBytes(unsigned char acDataEB);
		unsigned char getACDataEffectiveBytes();
		unsigned short incrementTSPCounter();
		void setTSPCounter(unsigned short counter);
		unsigned short getTSPCounter();
		void setACData(char* acData, unsigned short length);
		unsigned short getACData(char** acData);
		unsigned short getACDataLength();

		int updateStream();
		int getStream(char** dataStream);

};

}
}
}
}
}

#endif /* ISDBTINFORMATION_H_ */
