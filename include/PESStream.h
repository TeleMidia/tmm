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
 * PESStream.h
 *
 *  Created on: 25/03/2013
 *      Author: Felippe Nagato
 */

#ifndef PESSTREAM_H_
#define PESSTREAM_H_

#include "Stream.h"
#include "TSPacket.h"
#include "tsparser/Demuxer.h"
#include "PESPacket.h"
#include "project/inputdata/InputRange.h"
#include <iostream>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class PESStream : public Stream {

	private:

	protected:

		PESPacket* pesPacket;
		int64_t lastDtsAvailable;
		int64_t lastNextSendWithDts;

		bool isVideoStream;
		bool hasDts;
		vector<int64_t> ptsDtsDiff;
		vector<int64_t> periodList;
		vector<unsigned char> rangeChangeList;
		unsigned char rangeChange, lastRangeChange;
		bool isInRangeNow;
		Demuxer* demuxer;
		TSFileReader* tsReader;
		int64_t firstPts;
		int64_t timestamp1, timestamp2;
		int frameInterval;
		unsigned char streamId;

		vector<InputRange*>* inputRangeList;

		void addBuffer();
		bool timestampControl(PESPacket* packet);
		void resetControls();
		bool isInRange(int64_t pts);
		void processPesPacket();

	public:
		PESStream();
		virtual ~PESStream();

		void fillBuffer();
		bool disposeBuffer();
		bool getBuffer(Buffer** buffer);

		void updateNextSend(int64_t stc);
		void setMaxBitrate(unsigned int rate){};//cannot be set
		void setIsVideoStream(bool v);
		void setHasDts(bool has);
		void addPidFilter(unsigned short pid);
		void setFilename(const string& filename);
		void setStreamId(unsigned char id);

		void setInputRangeList(vector<InputRange*>* irl);

};

}
}
}
}


#endif /* PESSTREAM_H_ */
