/*
 * InputRange.h
 *
 *  Created on: 27/02/2013
 *      Author: Felippe Nagato
 */

#ifndef INPUTRANGE_H_
#define INPUTRANGE_H_

#include <iostream>
#include <map>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class InputRange {

	private:
		int id;
		int begin;
		int end;

	protected:

	public:
		InputRange();
		InputRange(int id);
		virtual ~InputRange();

		void setId(int id);
		void setBegin(int begin);
		void setEnd(int end);
		int getId();
		int getBegin();
		int getEnd();

};

}
}
}
}


#endif /* INPUTRANGE_H_ */
