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
 * PAit.cpp
 *
 *  Created on: 03/10/2013
 *      Author: Felippe Nagato
 */

#include "project/si/PAit.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

PAit::PAit() {
	ProjectInfo();
	projectType = PT_AIT;
	layer = 0x02; //HIERARCHY_B
	carouselProj = NULL;
}

PAit::~PAit() {

}

void PAit::setCarouselProj(ProjectInfo* proj) {
	carouselProj = proj;
}

ProjectInfo* PAit::getCarouselProj() {
	return carouselProj;
}

}
}
}
}





