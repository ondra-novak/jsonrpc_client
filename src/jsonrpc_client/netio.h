/*
 * netio.h
 *
 *  Created on: 10. 11. 2016
 *      Author: ondra
 */

#pragma once
#include <imtjson/refcnt.h>
#include <imtjson/stringview.h>
#include <stdint.h>

#include "abstractio.h"



namespace jsonrpc_client {


using json::StrViewA;


class NetworkConnection: public AbstractInputStream, public AbstractOutputStream {
public:

	static NetworkConnection *connect(const StrViewA &addr_ddot_port, int defaultPort);



	int getLastRecvError() const {
		return lastRecvError;
	}

	int getLastSendError() const {
		return lastSendError;
	}

	bool isTimeout() const {
		return timeout;
	}

	bool hasErrors() const {
		return timeout||lastRecvError||lastSendError;
	}




	void setTimeout(uintptr_t timeout);

	~NetworkConnection();

	virtual void closeOutput();
	virtual void closeInput();
	void close();




protected:

	NetworkConnection(int socket);

	virtual json::BinaryView doWrite(const json::BinaryView &data, bool nonblock = false);
	virtual bool doWaitWrite(int milisecs) ;
	virtual json::BinaryView doRead(bool nonblock = false);
	virtual bool doWaitRead(int milisecs) ;
	virtual Buffer createBuffer();


	void setNonBlock();
	void disableNagle();

	int socket;
	
	void *waitHandle; //<used by some platforms (Windows)

	unsigned char inputBuff[4096];
	unsigned char outputBuff[4096];
	bool eofFound;
	int lastSendError;
	int lastRecvError;
	bool timeout;
	uintptr_t timeoutTime;


	int readToBuff();

};

typedef json::RefCntPtr<NetworkConnection> PNetworkConection;




}


