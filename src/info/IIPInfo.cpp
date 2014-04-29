/*
 * IIPInfo.cpp
 *
 *  Created on: 25/04/2014
 *      Author: Felippe Nagato
 */

#include "info/IIPInfo.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

IIPInfo::IIPInfo() {

}

IIPInfo::~IIPInfo() {

}

bool IIPInfo::printTable() {
	ConfigurationInformation* ci;
	TransmissionParameters* tp;

	cout << "IIP - ISDB Information Packet" << endl <<
			"-----------------------------" << endl << endl;

	cout << "IIP packet pointer: " << IIPPacketPointer << endl;
	if (mcci) {
		cout << "Guard interval: ";
		switch (mcci->getCurrentGuardInterval()) {
		case MCCI_GUARD_INTERVAL_1_32:
			cout << "1/32";
			break;
		case MCCI_GUARD_INTERVAL_1_16:
			cout << "1/16";
			break;
		case MCCI_GUARD_INTERVAL_1_8:
			cout << "1/8";
			break;
		case MCCI_GUARD_INTERVAL_1_4:
			cout << "1/4";
			break;
		}
		cout << endl;
		cout << "Transmission mode: " << (mcci->getCurrentMode() & 0xFF) << endl;
		ci = mcci->getCurrentCI();
		if (ci) {
			cout << "Partial reception: ";
			if (ci->partialReceptionFlag) cout << "Yes"; else cout << "No";
			cout << endl;
			for (int i = 0; i < 3; ++i) {
				cout << endl;
				if (i == 0) tp = ci->tpLayerA;
				if (i == 1) tp = ci->tpLayerB;
				if (i == 2) tp = ci->tpLayerC;
				if (tp) {
					if (i == 0) cout << "- LAYER A";
					if (i == 1) cout << "- LAYER B";
					if (i == 2) cout << "- LAYER C";
					if (tp->modulationScheme == UNUSED_HIERARCHICAL_LAYER) {
						cout << " (Not used)" << endl;
					} else {
						cout << endl << "  Modulation scheme: ";
						switch (tp->modulationScheme) {
						case MCCI_MODULATION_SCHEME_DQPSK:
							cout << "DQPSK";
							break;
						case MCCI_MODULATION_SCHEME_QPSK:
							cout << "QPSK";
							break;
						case MCCI_MODULATION_SCHEME_16QAM:
							cout << "16QAM";
							break;
						case MCCI_MODULATION_SCHEME_64QAM:
							cout << "64QAM";
							break;
						}
						cout << endl;
						cout << "  Convolutional coding rate: ";
						switch (tp->codingRateOfInnerCode) {
						case MCCI_CONVOLUTIONAL_CODING_RATE_1_2:
							cout << "1/2";
							break;
						case MCCI_CONVOLUTIONAL_CODING_RATE_2_3:
							cout << "2/3";
							break;
						case MCCI_CONVOLUTIONAL_CODING_RATE_3_4:
							cout << "3/4";
							break;
						case MCCI_CONVOLUTIONAL_CODING_RATE_5_6:
							cout << "5/6";
							break;
						case MCCI_CONVOLUTIONAL_CODING_RATE_7_8:
							cout << "7/8";
							break;
						}
						cout << endl;
						cout << "  Length of time interleaving: " <<
								(tp->lengthOfTimeInterleaving & 0xFF) << endl;
						cout << "  Number of segments: " <<
								(tp->numberOfSegments & 0xFF) << endl;
					}
				}
			}

		}
	}

	return true;
}

}
}
}
}


