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
#include "ISDBTInformation.h"
#include "Stream.h"
#include "socket/MulticastServer.h"

#include <iostream>
#include <map>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;
using namespace br::pucrio::telemidia::tool::isdbt;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class Muxer {

	private:

	protected:
		vector<unsigned int> listOfAllPossiblePcrsFrequencies;
		map<unsigned short, unsigned int> pcrList;
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
		//int64_t packetCounter; //for debug purpose

		SocketServer* server;
		Stc stc_s; 		//for streaming output
		int64_t stc;   //for file output
		int64_t stcStep;
		char* streamBuffer;
		int streamBufferSize;
		int streamBufferLength;
		unsigned short packetsInBuffer;
		unsigned char ttl;

		unsigned int layerRateA;
		unsigned int layerRateB;
		unsigned int layerRateC;
		unsigned int pktNumSinceLastStepLayerA;
		unsigned int pktNumSinceLastStepLayerB;
		unsigned int pktNumSinceLastStepLayerC;
		unsigned int pktPerStepIntervalLayerA;
		unsigned int pktPerStepIntervalLayerB;
		unsigned int pktPerStepIntervalLayerC;

		unsigned int tsBitrate;
		unsigned char packetSize;
		string destination;
		bool isFileMode;
		map<unsigned short, vector<Stream*>*> streamList;

		bool odfmFrameEven;
		unsigned short ofdmFrameSize, ofdmFrameCounter;
		ISDBTInformation isdbtInfo;
		map<unsigned short, unsigned char> pidToLayerList;

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
											int *index,
											bool *streamScheduled);

		void resetPacketCounters();
		void newStcStep();
		int processNullPackets();
		int processPcr();
		int writeTsPcr(int64_t pcr, unsigned short pid);
		int writeTsStream(unsigned short pid, unsigned char type);
		int calculateBitrate();
		int writeStream(char* pktBuffer);
		int fillPacket204(char* stream, unsigned short pid);

	public:
		Muxer(unsigned char packetSize, unsigned short packetsInBuffer);
		virtual ~Muxer();

		//Greatest Common Divisor
		static unsigned int gcd(unsigned int a, unsigned int b);

		void setLayerRateA(unsigned int rate);
		unsigned int getLayerRateA();
		void setLayerRateB(unsigned int rate);
		unsigned int getLayerRateB();
		void setLayerRateC(unsigned int rate);
		unsigned int getLayerRateC();

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
		bool removeElementaryStream(unsigned short pid);
		bool removeAllElementaryStreams();
		bool addToListOfAllPossiblePcrsFrequencies(unsigned int freq);
		bool addPcrPid(unsigned short pid, unsigned int frequency); //pid,us
		bool removePcrPid(unsigned short pid);
		map<unsigned short, unsigned int>* getPcrList();

		map<unsigned short, vector<Stream*>*>* getStreamList();

		void setOfdmFrameSize(unsigned short size);
		bool addPidToLayer(unsigned short pid, unsigned char layer);
		map<unsigned short, unsigned char>* getPidToLayerList();
		void clearPidToLayerList();

		bool prepareMultiplexer(int64_t stcBegin);
		int mainLoop();

};

}
}
}
}


#endif /* MUXER_H_ */
