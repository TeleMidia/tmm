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

#include "socket/MulticastServer.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {
MulticastServer::MulticastServer(const char* group, int portNumber) {
	this->groupAddr = (char*) group;
	this->portNumber = portNumber;

	if (portNumber <= 1 || portNumber > 65535) {
		cout << "MulticastServer::MulticastServer Invalid port number!" << endl;
		return;
	}
    if (inet_pton(AF_INET, this->groupAddr, &mAddr.sin_addr) <= 0) {
        std::cerr << "MulticastServer::MulticastServer Invalid address format!" << std::endl;
        return; 
    }

	mAddr.sin_family = AF_INET;
    mAddr.sin_port = htons(this->portNumber);

	loopedBack = 0;
	ttl = 16;
}

MulticastServer::~MulticastServer() {

}

void MulticastServer::setTTL(unsigned char t) {
	ttl = t;
}

void MulticastServer::setLoopedBack(unsigned char lb) {
	loopedBack = lb;
}

bool MulticastServer::createSocket() {
	if (mAddr.sin_family != AF_INET) {
		return false;
	}
#ifdef WIN32
	WORD L_Ver;
	WSADATA wsaData;

	L_Ver = MAKEWORD( 2, 2);

	int L_Err = WSAStartup(L_Ver, &wsaData);
	if (L_Err != 0) {
		cout << "MulticastServer::createSocket Warning!";
		cout << " Can not find winsok dll!" << endl;
		return false;
	}

	if (LOBYTE(wsaData.wVersion) < 2 || HIBYTE(wsaData.wVersion) < 2) {
		cout << "MulticastServer::createSocket Warning!";
		cout << " Winsok dll is too old!" << endl;
		return false;
	}
#endif

	sockDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockDescriptor < 0) {
		perror("MulticastServer::createSocket");
		return false;
	}

	if (tryToBind()) {
		if (setSocketOptions()) {
			return true;
		}
	}

	cout << "MulticastServer::createSocket Warning!";
	cout << " return false" << endl;
	return false;
}

bool MulticastServer::tryToBind() {
	int ret;

	struct sockaddr_in stSourceAddr;

	stSourceAddr.sin_family = AF_INET;
	stSourceAddr.sin_port = htons(this->portNumber);
	stSourceAddr.sin_addr.s_addr = INADDR_ANY;

	/*
	* Calling bind() is not required, but some implementations need it
	* before you can reference any multicast socket options
	*/
	ret = bind(
			sockDescriptor,
			(struct sockaddr*)&stSourceAddr,
			sizeof(struct sockaddr));

	if (ret < 0) {
		perror ("MulticastServer::tryToBind bind");
		return false;
	}

	if (setsockopt(sockDescriptor, IPPROTO_IP, IP_MULTICAST_IF,
				   (char*)&stSourceAddr, sizeof(stSourceAddr)) < 0) {
		//perror ("MulticastServer::tryToBind IP_MULTICAST_IF");
	}

	return true;
}

bool MulticastServer::setSocketOptions() {
	int ret = setsockopt(sockDescriptor, IPPROTO_IP, IP_MULTICAST_LOOP,
						(char*)&loopedBack, sizeof(loopedBack));

	if (ret < 0) {
		perror("Multicast::setSocketOptions loop");
	}

	ret = setsockopt(sockDescriptor, IPPROTO_IP, IP_MULTICAST_TTL,
					(char*)&ttl, sizeof(ttl));

	if (ret < 0) {
		perror("Multicast::setSocketOptions ttl");
	}

	return true;
}

int MulticastServer::send(char* buff, unsigned int size) {
	int bytes;

	bytes = sendto(sockDescriptor, buff, size, 0, (struct sockaddr*)&mAddr,
				   sizeof(mAddr));

	if (bytes < 0) {
#ifdef WIN32
		cout << "MulticastServer::send '" << bytes << "' bytes";
		cout << ", requested '" << size << "'. Error: ";
		cout << WSAGetLastError() << endl;

#else
		herror ("MulticastServer::send");
#endif
	}

	return bytes;
}

}
}
}
}
