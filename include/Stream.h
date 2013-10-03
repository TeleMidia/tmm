/*
 * Stream.h
 *
 *  Created on: 05/02/2013
 *      Author: Felippe Nagato
 */

#ifndef STREAM_H_
#define STREAM_H_

#include "Stc.h"
#include "tsparser/TSFileReader.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

struct Buffer {
	 char* buffer;
	 unsigned int pos;
	 unsigned int size;
	 unsigned int originalSize;
	 bool startIndicator;
};

class Stream {

	private:

	protected:
		vector<Buffer*> bufferList;
		unsigned int maxBufferLength;

		int64_t nextSend;
		unsigned int frequency; //Value in STC unit (27 MHz).
		unsigned int maxBitrate;
		unsigned int maxBytesRate;

		//0 - PES or IIP
		//1 - Section
		unsigned char type;

		unsigned int preponeTicks;

		int64_t currStc;

		//TODO: add a flag indicating that the buffer should not be discarded
		//		in disposeBuffer() function.

	public:

		Stream();
		virtual ~Stream();

		virtual bool getBuffer(Buffer** buffer);
		virtual unsigned int getBufferSize();
		virtual bool disposeBuffer();
		virtual void setMaxBufferLength(unsigned int max);
		virtual unsigned int getMaxBufferLength();
		virtual void releaseBufferList();
		virtual void fillBuffer()=0;

		virtual void initiateNextSend(int64_t stc);
		virtual void updateNextSend(int64_t stc);
		virtual int64_t getNextSend();
		virtual void setFrequency(unsigned int freq); //27 MHz (max. = 159s)
		virtual unsigned int getFrequency();
		virtual void setMaxBitrate(unsigned int rate);
		virtual unsigned int getMaxBitrate();

		virtual void setType(unsigned char type);
		virtual unsigned char getType();

		virtual void setPreponeTicks(unsigned int ticks);
		virtual unsigned int getPreponeTicks();

		virtual void setCurrStc(int64_t stc);

};

}
}
}
}

#endif /* STREAM_H_ */
