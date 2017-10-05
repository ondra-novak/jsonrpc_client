/*
 * exception.cpp
 *
 *  Created on: 9. 3. 2016
 *      Author: ondra
 */

#include "exception.h"

#include <sstream>

namespace jsonrpc_client {

String SystemException::getWhatMsg() const throw () {
	std::ostringstream buff;
	buff << StrViewA(text) << " errno=" << errn;
	return String(buff.str());
}

const char* Exception::what() const throw () {
	if (whatMsg.empty()) {
		whatMsg = getWhatMsg();
	}
	return whatMsg.c_str();
}


}

