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
 * PPat.h
 *
 *  Created on: 04/09/2013
 *      Author: Felippe Nagato
 */

#ifndef PPAT_H_
#define PPAT_H_

#include "project/ProjectInfo.h"
#include "si/Pat.h"
#include <iostream>

using namespace std;
using namespace br::pucrio::telemidia::mpeg2;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

class PPat : public ProjectInfo, public Pat {

	private:

	protected:

	public:
		PPat(){
			ProjectInfo();
			projectType = PT_PAT;
		};
		virtual ~PPat(){};

};

}
}
}
}


#endif /* PPAT_H_ */
