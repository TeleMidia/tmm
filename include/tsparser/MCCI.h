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
 * MCCI.h
 *
 *  Created on: 25/09/2013
 *      Author: Felippe Nagato
 */

// Modulation Control Configuration Information

#ifndef MCCI_H_
#define MCCI_H_

#include "Crc32.h"
#include <iostream>
#include <cstring>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {
namespace isdbt {

struct TransmissionParameters {
	unsigned char modulationScheme;
	unsigned char codingRateOfInnerCode;
	unsigned char lengthOfTimeInterleaving;
	unsigned char numberOfSegments;
};

struct ConfigurationInformation {
	bool partialReceptionFlag;
	TransmissionParameters* tpLayerA;
	TransmissionParameters* tpLayerB;
	TransmissionParameters* tpLayerC;
};

class MCCI {
	#define UNUSED_HIERARCHICAL_LAYER 0x07

	#define MCCI_MODULATION_SCHEME_DQPSK 0x00
	#define MCCI_MODULATION_SCHEME_QPSK  0x01
	#define MCCI_MODULATION_SCHEME_16QAM 0x02
	#define MCCI_MODULATION_SCHEME_64QAM 0x03

	#define MCCI_CONVOLUTIONAL_CODING_RATE_1_2 0x00
	#define MCCI_CONVOLUTIONAL_CODING_RATE_2_3 0x01
	#define MCCI_CONVOLUTIONAL_CODING_RATE_3_4 0x02
	#define MCCI_CONVOLUTIONAL_CODING_RATE_5_6 0x03
	#define MCCI_CONVOLUTIONAL_CODING_RATE_7_8 0x04

	#define MCCI_GUARD_INTERVAL_1_32 0x00
	#define MCCI_GUARD_INTERVAL_1_16 0x01
	#define MCCI_GUARD_INTERVAL_1_8  0x02
	#define MCCI_GUARD_INTERVAL_1_4  0x03

	#define MCCI_TRANSMISSION_MODE_1 0x01
	#define MCCI_TRANSMISSION_MODE_2 0x02
	#define MCCI_TRANSMISSION_MODE_3 0x03

	private:

	protected:
		char* stream;

		unsigned char TMCCSynchronizationWord;
		unsigned char ACDataEffectivePosition;
		unsigned char initializationTimingIndicator;
		unsigned char currentMode;
		unsigned char currentGuardInterval;
		unsigned char nextMode;
		unsigned char nextGuardInterval;
		unsigned char systemIdentifier;
		unsigned char countDownIndex;
		bool switchOnControlFlag;
		ConfigurationInformation* currentCI;
		ConfigurationInformation* nextCI;
		unsigned char phaseCorrection;
		unsigned int crc32;

		void init();

	public:
		MCCI();
		MCCI(char* data);
		virtual ~MCCI();

		void setTMCCSynchronizationWord(unsigned char sw);
		void setACDataEffectivePosition(unsigned char position);
		void setInitializationTimingIndicator(unsigned char indicator);
		void setCurrentMode(unsigned char mode);
		void setCurrentGuardInterval(unsigned char interval);
		void setNextMode(unsigned char mode);
		void setNextGuardInterval(unsigned char interval);
		void setSystemIdentifier(unsigned char identifier);
		void setCountDownIndex(unsigned char index);
		void setSwitchOnControlFlag(unsigned char flag);
		void setCurrentCI(ConfigurationInformation* ci);
		void setNextCI(ConfigurationInformation* ci);
		void setPhaseCorrection(unsigned char phase);

		unsigned char getTMCCSynchronizationWord();
		unsigned char getACDataEffectivePosition();
		unsigned char getInitializationTimingIndicator();
		unsigned char getCurrentMode();
		unsigned char getCurrentGuardInterval();
		unsigned char getNextMode();
		unsigned char getNextGuardInterval();
		unsigned char getSystemIdentifier();
		unsigned char getCountDownIndex();
		bool getSwitchOnControlFlag();
		ConfigurationInformation* getCurrentCI();
		ConfigurationInformation* getNextCI();
		unsigned char getPhaseCorrection();
		unsigned int getCrc32();

		bool copyCurrentToNext();

		int updateStream();
		int getStream(char** dataStream);

};

}
}
}
}
}


#endif /* MCCI_H_ */
