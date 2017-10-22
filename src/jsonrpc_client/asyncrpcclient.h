#pragma once
#include "netio.h"
#include <imtjson/rpc.h>
#include <thread>
#include "netio.h"

namespace jsonrpc_client {

using namespace json;



class RpcClient: public json::AbstractRpcClient {
public:

	typedef json::AbstractRpcClient Super;

	RpcClient();
	~RpcClient();


	virtual void sendRequest(Value request);

	virtual void onInit() {}
	virtual void onNotify(const Notify &ntf) {}
	virtual void logError(const json::Value &details) {}
	virtual void logTrafic(bool received, const json::Value &details) {}

	String getAddr() const {return addr;}

	///causes disconnecting the client
	/** useful when error found to reconnect later. Note that function is asynchronous, There
	 * still can be request will be processed after the disconect is requested.
	 */
	void disconnect(bool sync = false);
	///Connects stream if necesery. Operation may be asynchronous
	bool  connect(StrViewA addr);

	bool isConnected() const {return connected;}
	void setRecvTimeout(int timeoutms);

protected:

	void worker(PNetworkConection conn);


	String addr;

	int rcvtimeout = -1;

	jsonrpc_client::PNetworkConection conn;


	std::thread workerThr;
	bool connected;

private:
	void sendJSON(const Value& v);
	void connectLk(StrViewA addr);
};



}
