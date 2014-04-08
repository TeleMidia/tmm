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
