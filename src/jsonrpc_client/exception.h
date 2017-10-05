
#pragma once
#include <imtjson/string.h>

namespace jsonrpc_client {

using namespace json;

class Exception: public std::exception {
public:

	virtual const char *what() const throw() override;
	virtual ~Exception() throw() {}

protected:
	mutable String whatMsg;
	virtual String getWhatMsg() const throw() = 0;

};

class SystemException: public Exception {
public:

	SystemException(String text, int errn):text(text),errn(errn) {}
	int getErrorNo() const {return errn;}
protected:
	String text;
	int errn;
	virtual String getWhatMsg() const throw();

};


}




