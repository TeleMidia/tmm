/*
 * ProjectInfo.h
 *
 *  Created on: 07/03/2013
 *      Author: Felippe Nagato
 */

#ifndef PROJECTINFO_H_
#define PROJECTINFO_H_

#include <inttypes.h>
#include <iostream>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class ProjectInfo {

	#define PT_UNKNOWN 0x00
	#define PT_INPUTDATA 0x01
	#define PT_NPT 0x02
	#define PT_STREAMEVENT 0x03
	#define PT_CAROUSEL 0x04
	#define PT_AIT 0x05
	#define PT_TOT 0x06
	#define PT_PMTVIEW 0x07
	#define PT_TIMELINE 0x08
	#define PT_SDT 0x09
	#define PT_NIT 0x0A
	#define PT_PES 0xFE
	#define PT_SECTION 0xFF

	private:

	protected:
		int id;
		char projectType;
		unsigned char version;

	public:
		ProjectInfo();
		virtual ~ProjectInfo();

		void setId(int id);
		int getId();
		void setProjectType(char type);
		char getProjectType();
		void setVersion(unsigned char ver);
		unsigned char getVersion();

};

}
}
}
}


#endif /* PROJECTINFO_H_ */
