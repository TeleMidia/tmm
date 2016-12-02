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

#ifndef MULTICASTSERVER_H_
#define MULTICASTSERVER_H_

#include "socket/SocketServer.h"

#include <iostream>
#include <fstream>
using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {
	class MulticastServer : public SocketServer {
		private:
			char* groupAddr;
			struct sockaddr_in mAddr;
			unsigned char loopedBack;
			bool tryToBind();
			bool setSocketOptions();

		public:
			MulticastServer(const char* group, int port);
			virtual ~MulticastServer();

			bool createSocket();
			int send(char* buff, unsigned int size);
			void setTTL(unsigned char t);
			void setLoopedBack(unsigned char lb);
	};
}
}
}
}

#endif /*MULTICASTSERVER_H_*/
