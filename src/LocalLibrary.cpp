/*
 * LocalLibrary.cpp
 *
 *  Created on: 04/02/2014
 *      Author: Felippe Nagato
 */

#include "LocalLibrary.h"
#include <iostream>

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace tool {

LocalLibrary::LocalLibrary() {

}

LocalLibrary::~LocalLibrary() {

}

bool LocalLibrary::executeApp(const string& filename, string parameters,
							  unsigned int* pid) {
	*pid = 0;
#ifdef _WIN32
	PROCESS_INFORMATION ProcessInfo; //This is what we get as an [out] parameter
	STARTUPINFO StartupInfo; //This is an [in] parameter
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof StartupInfo ; //Only compulsory field
	parameters = filename + ' ' + parameters;
	if(CreateProcess(filename.c_str(), (char*) parameters.c_str(),
	    NULL,NULL,FALSE,0,NULL,
	    NULL,&StartupInfo,&ProcessInfo)) {
		*pid = (unsigned int) ProcessInfo.dwProcessId;
	    return true;
	}
	else {
		DWORD dw = GetLastError();
		cout << "Error " << dw << endl;
	    return false;
	}
#else
	pid_t mpid;
	mpid = fork();
	if(fork == 0) {
		char* argv[5];
		argv[0] = NULL;
		execvp(filename.c_str(), argv);
		*pid = (unsigned int) mpid;
		return true;
	}
	else {
		return false;
	}
#endif
}

bool LocalLibrary::killApp(unsigned int pid) {
#ifdef _WIN32
	DWORD dwDesiredAccess = PROCESS_TERMINATE;
	BOOL bInheritHandle  = FALSE;
	unsigned int uExitCode = 0;
	HANDLE hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, (DWORD)pid);
	if (hProcess == NULL) return FALSE;
	BOOL result = TerminateProcess(hProcess, uExitCode);
	CloseHandle(hProcess);
	return result;
#else
	kill(pid, SIGTERM);
#endif
}

string LocalLibrary::getAttribute(XMLElement* e, const string& name) {
	string str = "";
	if (e) {
		const char *s = e->Attribute(name.c_str());
		if (s) str.assign(s);
	}
	return str;
}

string LocalLibrary::getElementText(XMLElement* e) {
	string str;
	XMLText* text = e->FirstChild()->ToText();
	str.assign(text->Value());
	return str;
}

string LocalLibrary::extractBaseId(const string& filename) {
	XMLDocument xmldoc;
	enum XMLError err;
	XMLElement *e;
	string value = "";

	if (filename.empty()) return "";
	err = xmldoc.LoadFile(filename.c_str());
	if (err != XML_SUCCESS) {
		return "";
	}

	e = xmldoc.FirstChildElement("ncl");
	if (e) value = getAttribute(e, "id");

	return value;
}

string LocalLibrary::upperCase(const string& text) {
	string ret;
	char letter;
	for (unsigned int i = 0; i < text.size(); i++) {
		letter = text[i];
		letter = toupper(letter);
		ret.append(&letter, 1);
	}
	return ret;
}

}
}
}
}
