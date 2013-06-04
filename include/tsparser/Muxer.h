/*
 * Muxer.h
 *
 *  Created on: 05/02/2013
 *      Author: Felippe Nagato
 */

#ifndef MUXER_H_
#define MUXER_H_

#include "Stc.h"
#include "TSPacket.h"
#include "Stream.h"
#include "socket/MulticastServer.h"

#include <iostream>
#include <map>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class Muxer {

	#define TOTAL_PACKETS_IN_BUFFER 40;

	private:

	protected:
		vector<unsigned short> listOfAllPossiblePcrsFrequencies;
		map<unsigned short, unsigned short> pcrList;
		map<unsigned short, int64_t> nextPcrSendList;
		unsigned int pcrFrequency;

		unsigned short minorPcr;
		unsigned short pcrPid;
		unsigned int pktPerStepInterval;
		unsigned int pktPerSec;
		double pktStc;
		unsigned int pktNumSinceLastStep;
		int64_t stcOffset;
		int64_t stcBegin;
		char pktControl[8192];

		SocketServer* server;
		Stc stc_s; 		//for streaming output
		int64_t stc;   //for file output
		int64_t stcStep;
		char* streamBuffer;
		int streamBufferSize;
		int streamBufferLength;
		unsigned char ttl;

		unsigned int tsBitrate;
		unsigned char packetSize;
		string destination;
		bool isFileMode;
		map<unsigned short, Stream*> streamList;

		Buffer* buffer;

		unsigned int bitrateErrorCounter;

		FILE * pFile;
		int open();
		int close();
		int sendStreamBuffer();
		bool calculateStcStep();
		int reachedNextPcrSend(int64_t stc);
		void fillAllBufferes();
		int64_t getNextStcStreamToMultiplex(unsigned short *pid,
											 bool *streamScheduled);

		void newStcStep();
		int processNullPackets();
		int processPcr();
		int writeTsPcr(int64_t pcr, unsigned short pid);
		int writeTsStream(unsigned short pid, unsigned char type);
		void calculateBitrate();
		int writeStream(char* pktBuffer);

	public:
		Muxer();
		virtual ~Muxer();

		//Greatest Common Divisor
		static unsigned int gcd(unsigned int a, unsigned int b);

		void setTsBitrate(unsigned int rate);
		unsigned int getTsBitrate();
		void setPacketSize(unsigned char size);
		unsigned char getPacketSize();
		void setDestination(string dest);
		string getDestination();
		void setStcReference(int64_t stcBegin);
		int64_t getCurrentStc();
		int64_t getStcBegin();
		int64_t getRelativeStc();
		double getPktStc();
		void setTTL(unsigned char t);

		bool addElementaryStream(unsigned short pid, Stream* stream);
		Stream* removeElementaryStream(unsigned short esPid);
		bool removeAllElementaryStreams();
		bool addToListOfAllPossiblePcrsFrequencies(unsigned short freq);
		bool addPcrPid(unsigned short pid, unsigned short frequency); //pid,ms
		bool removePcrPid(unsigned short pid);
		map<unsigned short, unsigned short>* getPcrList();

		map<unsigned short, Stream*>* getStreamList();

		bool prepareMultiplexer(int64_t stcBegin);
		int mainLoop();

};

}
}
}
}


#endif /* MUXER_H_ */
