/*
 * TMM.h
 *
 *  Created on: 27/02/2013
 *      Author: Felippe Nagato
 */

#ifndef TMM_H_
#define TMM_H_

#include "util/functions.h"
#include "project/Project.h"
#include "tsparser/TSFileReader.h"
#include "tsparser/TSInfo.h"
#include "tsparser/Muxer.h"
#include "PESStream.h"
#include "SectionStream.h"
#include "si/descriptor/CarouselIdentifier.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class TMM {
	#define PREPONETICKS_AUDIO 0.25
	#define PREPONETICKS_VIDEO 1.00

	private:

	protected:
		Project *project;
		Muxer* muxer;
		string destination;
		map<unsigned short, unsigned short> tempPcrsInUse;
		SectionStream* patStream;
		SectionStream* totStream;
		SectionStream* sdtStream;
		bool useSdt;
		unsigned char patVersion;
		unsigned char sdtVersion;
		unsigned int lastStcPrinter;

		ProjectInfo* getFirstProject(char projectType);
		ProjectInfo* getFirstProjectReversed(char projectType);
		bool loadProject();
		Stream *createStream(ProjectInfo* proj);
		bool createStreamList(vector<pmtViewInfo*>* currentTimeline,
							  vector<pmtViewInfo*>* newTimeline);

		void processPcrsInUse();
		int updateSdt(vector<pmtViewInfo*>* newTimeline, Sdt** sdt);
		int createPmt(PMTView* pmtView, Pmt** pmt);
		int createSiTables(vector<pmtViewInfo*>* newTimeline);
		int restoreSiTables(vector<pmtViewInfo*>* currentTimeline,
							vector<pmtViewInfo*>* newTimeline);

		int multiplex();
		int getTSInfo(InputData *inputList);
		int multiplexSetup();

	public:
		TMM();
		TMM(Project* project);
		virtual ~TMM();

		void setProject(Project* project);

		int sendTo(const char* destination);

};

}
}
}
}


#endif /* TMM_H_ */
