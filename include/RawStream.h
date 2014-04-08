/*
 * RawStream.h
 *
 *  Created on: 26/09/2013
 *      Author: Felippe Nagato
 */

#ifndef RAWSTREAM_H_
#define RAWSTREAM_H_

#include "Stream.h"
#include "Dependents.h"
#include "PrivateSection.h"
#include <vector>
#include <iostream>
#include <cstring>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class RawStream : public Stream, public Publisher {

	private:

	protected:
		vector<pair<char*,int>*> blockList;
		unsigned int currPos;
		bool destroyBlocks;

	public:
		RawStream();
		virtual ~RawStream();

		bool getBuffer(Buffer** buffer);
		void fillBuffer();

		void releaseBlockList();
		void setDestroyBlocks(bool destroy);

		bool addBlock(char* stream, int length);
		void addSection(PrivateSection* sec);
		bool addSection(const string& filename);

};

}
}
}
}


#endif /* ANYSTREAM_H_ */
