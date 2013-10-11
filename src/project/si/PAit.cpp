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





