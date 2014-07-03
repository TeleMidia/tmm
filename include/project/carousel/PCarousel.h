/*
 * PCarousel.h
 *
 *  Created on: 25/03/2013
 *      Author: Felippe Nagato
 */

#ifndef PCAROUSEL_H_
#define PCAROUSEL_H_

#include "project/ProjectInfo.h"
#include "dsmcc/carousel/Carousel.h"
#include <iostream>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2::dsmcc;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class PCarousel : public ProjectInfo, public Carousel {

	private:

	protected:
		string outputFile;
		string tempFolder;
		bool mounted;
	public:
		PCarousel();
		virtual ~PCarousel();

		void setOutputFile(const string& filename);
		string getOutputFile();
		void setTempFolder(const string& folder);
		string getTempFolder();
		void setMounted(bool mounted);
		bool getMounted();

};

}
}
}
}


#endif /* PCAROUSEL_H_ */
