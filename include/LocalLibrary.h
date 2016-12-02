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
 * LocalLibrary.h
 *
 *  Created on: 04/02/2014
 *      Author: Felippe Nagato
 */

#ifndef LOCALLIBRARY_H_
#define LOCALLIBRARY_H_

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <string>
#include <cctype>

#include "project/tinyxml2.h"

using namespace std;
using namespace tinyxml2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class LocalLibrary {

	private:

	protected:

	public:
		LocalLibrary();
		virtual ~LocalLibrary();

		static bool executeApp(const string& filename, string parameters,
							   unsigned int* pid);
		static bool killApp(unsigned int pid);

		static string getAttribute(XMLElement* e, const string& name);
		static string getElementText(XMLElement* e);
		static string extractBaseId(const string& filename);
		static string upperCase(const string& text);

};

}
}
}
}


#endif /* LOCALLIBRARY_H_ */
