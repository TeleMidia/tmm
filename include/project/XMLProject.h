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
 * XMLProject.h
 *
 *  Created on: 05/02/2013
 *      Author: Felippe Nagato
 */

#ifndef XMLPROJECT_H_
#define XMLPROJECT_H_

#include "project/Project.h"
#include "project/tinyxml2.h"

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cctype>

using namespace tinyxml2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

struct MetaData {
	 string name;
	 string content;
};

class XMLProject : public Project {
	#define TOT_NAME "_tot_project_"
	#define SDT_NAME "_sdt_project_"
	#define NIT_NAME "_nit_project_"
	#define TIMELINE_NAME "_timeline_project_"
	#define IIP_NAME "_iip_project_"

	private:
		XMLDocument* xmldoc;

		void init();

	protected:
		vector<MetaData*> metaDataList;
		map<string, int> idList;
		int idIndex;
		PSdt* pSdt;

		bool createNewId(const string& id);
		int getId(const string& id);
		int createAndGetId(ProjectInfo* proj, const string& name);

		int parseAV(XMLNode* m, XMLElement* f);
		int parseNPT(XMLNode* m,XMLElement* f);
		int parseStreamEvent(XMLNode* m,XMLElement* f);
		int parseCarousel(XMLNode* m, XMLElement* f);
		int parseAIT(XMLNode* m, XMLElement* f);
		int parseEIT(XMLNode* m, XMLElement* f);
		int parsePMT(XMLNode* m, XMLElement* f);

		int processInputs(XMLElement *top);
		int processOutputProperties(XMLElement *top);
		int processOutput(XMLElement *top);

		int readHead(XMLElement *top);
		int readBody(XMLElement *top);

	public:
		XMLProject();
		XMLProject(const string& filename);
		XMLProject(const char *filename);
		virtual ~XMLProject();

		vector<MetaData*>* getMetaDataList();

		int readFile();
		int readLiveStream();

};

}
}
}
}


#endif /* XMLPROJECT_H_ */
