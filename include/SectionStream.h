/*
 * SectionStream.h
 *
 *  Created on: 25/03/2013
 *      Author: Felippe Nagato
 */

#ifndef SECTIONSTREAM_H_
#define SECTIONSTREAM_H_

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

class SectionStream : public Stream, public Publisher {

	private:

	protected:
		vector<PrivateSection*> sectionList;
		unsigned int currPos;

	public:
		SectionStream();
		virtual ~SectionStream();

		bool getBuffer(Buffer** buffer);
		void fillBuffer();

		void releaseSectionList();

		void addSection(PrivateSection* sec);
		bool addSection(char* stream, int length);
		bool addSection(string filename);

};

}
}
}
}


#endif /* SECTIONSTREAM_H_ */
