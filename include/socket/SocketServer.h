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
