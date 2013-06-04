/*
 * TSInfo.h
 *
 *  Created on: 12/03/2013
 *      Author: Felippe Nagato
 */

#ifndef TSINFO_H_
#define TSINFO_H_

#include "TSPacket.h"
#include "PESPacket.h"
#include "si/Pat.h"
#include "si/Pmt.h"
#include "tsparser/TSFileReader.h"
#include "tsparser/Demuxer.h"
#include "Stc.h"
#include <iostream>
#include <string>
#include <map>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class TSInfo {

	private:
		Pat* pat;
		map<unsigned short,Pmt*> pmtList;
		TSFileReader* tsReader;
		int64_t firstPts;

		void clearPmtList();
		bool checkPmtsCount();

	protected:

	public:
		TSInfo();
		TSInfo(TSFileReader* tsReader);
		virtual ~TSInfo();

		void setTSFileReader(TSFileReader* tsReader);
		bool readInfo();
		unsigned char getStreamType(unsigned short pid);
		char hasDts(unsigned short pid, unsigned int pktCount);
		double duration(unsigned short pid);
		int64_t getFirstPts();

		void printTable();

		static bool isAudioStreamType(unsigned char st);
		static bool isVideoStreamType(unsigned char st);

};

}
}
}
}


#endif /* TSINFO_H_ */
