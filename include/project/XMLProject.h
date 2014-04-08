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

};

}
}
}
}


#endif /* XMLPROJECT_H_ */
