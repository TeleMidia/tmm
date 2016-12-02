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
 * InputData.h
 *
 *  Created on: 27/02/2013
 *      Author: Felippe Nagato
 */

#ifndef INPUTDATA_H_
#define INPUTDATA_H_

#include "project/ProjectInfo.h"
#include "InputRange.h"

#include <inttypes.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class InputData : public ProjectInfo {

	private:
		string filename;
		unsigned short pid;
		unsigned char streamType;
		bool hasDts;
		int offset;
		int64_t firstPts;
		double duration;
		vector<InputRange*>* inputRangeList;

	protected:

	public:
		InputData();
		InputData(int id);
		virtual ~InputData();

		void setFilename(const string& filename);
		string getFilename();
		void setPid(unsigned short pid);
		unsigned short getPid();
		void setStreamType(unsigned char st);
		unsigned short getStreamType();
		void setHasDts(bool has);
		bool getHasDts();
		void setOffset(int os);
		int getOffset();
		void setFirstPts(int64_t pts);
		int64_t getFirstPts();
		void setDuration(double dur);
		double getDuration();
		bool addRange(InputRange* ir);
		bool addRange(int id, int64_t begin, int64_t end);
		bool removeRange(int id);
		bool getRange(int id, int64_t* begin, int64_t *end);
		vector<InputRange*>* getInputRangeList();

};

}
}
}
}


#endif /* INPUTDATA_H_ */
