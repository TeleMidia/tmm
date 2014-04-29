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
#include "info/si/PatInfo.h"
#include "info/si/PmtInfo.h"
#include "info/si/TotInfo.h"
#include "info/si/SdtInfo.h"
#include "info/si/NitInfo.h"
#include "info/IIPInfo.h"
#include "tsparser/ISDBTInformation.h"
#include "tsparser/TSFileReader.h"
#include "tsparser/Demuxer.h"
#include "Stc.h"

#include <inttypes.h>
#include <iostream>
#include <string>
#include <map>
#include <set>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class TSInfo {

	private:
		PatInfo* pat;
		TotInfo* tot;
		SdtInfo* sdt;
		NitInfo* nit;
		IIPInfo* iip;
		map<unsigned short,PmtInfo*> pmtList;
		map<unsigned short, set<unsigned char>* > layerList;
		TSFileReader* tsReader;
		int64_t firstPts;

		void clearPmtList();
		bool checkPmtsCount();
		void init();
		bool processPacket(TSPacket *packet, PrivateSection **section);

	protected:
		unsigned int packetCounter;

	public:
		TSInfo();
		TSInfo(TSFileReader* tsReader);
		virtual ~TSInfo();

		void setTSFileReader(TSFileReader* tsReader);
		bool readInfo(unsigned char mode);
		unsigned char getStreamType(unsigned short pid);
		char hasDts(unsigned short pid, unsigned int pktCount);
		double duration(unsigned short pid);
		int64_t getFirstPts();

		void printESTable();
		bool printTables();

		static bool isAudioStreamType(unsigned char st);
		static bool isVideoStreamType(unsigned char st);

};

}
}
}
}


#endif /* TSINFO_H_ */
