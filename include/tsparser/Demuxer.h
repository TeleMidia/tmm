/*
 * Demuxer.h
 *
 *  Created on: 05/02/2013
 *      Author: Felippe Nagato
 */

#ifndef DEMUXER_H_
#define DEMUXER_H_

#include "TSPacket.h"
#include "tsparser/TSFileReader.h"
#include <iostream>
#include <set>
#include <map>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class Demuxer {

	private:
		TSFileReader* tsReader;
		set<unsigned short> *pidFilterList;
		map<unsigned short, char> continuityCounterList;

	protected:

	public:
		Demuxer();
		Demuxer(TSFileReader* tsReader);
		virtual ~Demuxer();

		void setTsReader(TSFileReader* tsReader);
		void addPidFilter(unsigned short pid);
		void removePidFilter(unsigned short pid);
		int getNextPacketbyFilter(TSPacket **packet);
		void setContinuityCounter(unsigned short pid, char cc);
		char getContinuityCounter(unsigned short pid);


};

}
}
}
}

#endif /* DEMUXER_H_ */
