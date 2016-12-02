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
		int projectId;

		vector<Buffer*> bufferList;
		unsigned int maxBufferLength;

		int64_t nextSend;
		unsigned int period; //Value in STC unit (27 MHz).
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

		void setProjectId(int id);
		int getProjectId();

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
		virtual void setPeriod(unsigned int freq); //27 MHz (max. = 159s)
		virtual unsigned int getPeriod();
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
