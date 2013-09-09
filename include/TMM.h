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
#include "si/descriptor/StreamIdentifier.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class TMM {
	#define PREPONETICKS_AUDIO 0.4
	#define PREPONETICKS_VIDEO 0.5

	private:
		void init();
		void releaseSiStreamList();

	protected:
		Project *project;
		Muxer* muxer;
		string destination;
		map<ProjectInfo*, SectionStream*> siStreamList;
		unsigned int lastStcPrinter;

		ProjectInfo* getFirstProject(char projectType);
		ProjectInfo* getFirstProjectReversed(char projectType);
		bool loadProject();
		Stream *createStream(ProjectInfo* proj);
		bool createStreamList(vector<pmtViewInfo*>* currentTimeline,
							  vector<pmtViewInfo*>* newTimeline);
		bool releaseStreamFromList(ProjectInfo* proj);

		void processPcrsInUse(vector<pmtViewInfo*>* newTimeline);
		int createPmt(PMTView* currentPmtView, PMTView* newPmtView, Pmt** pmt);
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
