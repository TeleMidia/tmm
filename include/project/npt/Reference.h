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
