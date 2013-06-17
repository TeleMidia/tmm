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
