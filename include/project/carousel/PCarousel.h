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
