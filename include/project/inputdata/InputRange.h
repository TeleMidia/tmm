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
