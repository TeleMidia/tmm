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

#ifndef SOCKETSERVER_H_
#define SOCKETSERVER_H_

#ifndef WIN32
  #ifdef _MSC_VER
    #define WIN32
  #endif
#endif

#ifdef WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <sys/ioctl.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <net/if.h>
#endif

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {
	class SocketServer {
		protected:
#ifdef WIN32
			SOCKET sockDescriptor;
#else
			int sockDescriptor;
#endif
			unsigned int portNumber;
			unsigned char ttl;

		public:
			virtual ~SocketServer(){};
			virtual bool createSocket()=0;
			virtual int send(char* buff, unsigned int size)=0;
			virtual void setTTL(unsigned char t)=0;

	};
}
}
}
}

#endif /*SOCKETSERVER_H_*/
