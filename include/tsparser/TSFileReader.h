/*
 * TSFileReader.h
 *
 *  Created on: 05/02/2013
 *      Author: Felippe Nagato
 */

#ifndef TSFILEREADER_H_
#define TSFILEREADER_H_

#include <inttypes.h>
#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class TSFileReader {
	#define BUFFER_SIZE 204 * 5000

	private:
		FILE * pFile;
		char* buffer;
		int bufferPos;
		int bufferLength;
		unsigned char packetSize;
		int loopCount;
		string filename;
		int64_t length;

		int read();
		int open();
		int close();

	protected:
		int searchSyncByte();

	public:
		TSFileReader();
		TSFileReader(const string& filename);
		virtual ~TSFileReader();

		void setFilename(const string& filename);

		int getBuffer(char** buffer);
		int getLoopCount();
		bool rewind();
		int64_t getLength();
		void resetLoopCount();
		int goTo(int64_t position);
		unsigned char getPacketSize();

		static int readPacketSize(const string& filename);

};

}
}
}
}


#endif /* TSFILEREADER_H_ */
