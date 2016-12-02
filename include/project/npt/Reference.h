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
 * Reference.h
 *
 *  Created on: 28/02/2013
 *      Author: Felippe Nagato
 */

#ifndef REFERENCE_H_
#define REFERENCE_H_

#include <inttypes.h>
#include <iostream>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class Reference {

	private:
		int id;
		char contentId;
		int64_t absStart;
		int64_t absEnd;
		int64_t nptStart;
		int64_t nptEnd;
		short numerator;
		unsigned short denominator;
		bool hasNext;
		bool inTransition;
		bool inInterval;

	protected:

	public:
		Reference();
		virtual ~Reference();

		void setId(int id);
		int getId();
		void setContentId(unsigned char id);
		unsigned char getContentId();
		void setAbsStart(int64_t start);
		int64_t getAbsStart();
		void setAbsEnd(int64_t end);
		int64_t getAbsEnd();
		void setNptStart(int64_t start);
		int64_t getNptStart();
		void setNptEnd(int64_t end);
		int64_t getNptEnd();
		void setNumerator(short num);
		short getNumerator();
		void setDenominator(unsigned short den);
		unsigned short getDenominator();
		void setHasNext(bool hasNext);
		bool getHasNext();
		void setInTransition(bool transition);
		bool getInTransition();
		void setInInterval(bool interval);
		bool getInInterval();

};

}
}
}
}


#endif /* REFERENCE_H_ */
