/*
 * main.cpp
 *
 *  Created on: Jul 28, 2017
 *      Author: ondra
 */

#include <unistd.h>	//for isatty()
#include <stdio.h>	//for fileno()
#include <iostream>
#include <imtjson/string.h>
#include <imtjson/parser.h>
#include <imtjson/serializer.h>

#include <jsonrpc_client/asyncrpcclient.h>

using namespace json;

void putSpaces(std::ostream &stream, int spaces) {
	stream<< std::endl;
	for (unsigned int i = 0; i < spaces; i++) stream << "  ";
}

void simpleFmtString(std::ostream &stream, StrViewA string, int spaces) {
	stream << '"';
	for (char c: string) {
		if (c == '\n') {
			putSpaces(stream,spaces);
		} else if (c < 32) {stream << '.';
	 	} else if (c == '"') stream << "\\\"";
		else stream << c;
	}
	stream << '"';
}

void simpleFmt(std::ostream &stream, Value json, int spaces = 0) {
	switch (json.type()) {
	case json::string: simpleFmtString(stream, json.getString(), spaces);break;
		default:  stream << json.toString();break;
		case json::array: {
			char sep = '[';
			for (Value v : json) {
				stream << sep << ' ';
				if (v.type() == json::array) {
					putSpaces(stream, spaces+1);
					simpleFmt(stream, v, spaces+2);
				} else {
					simpleFmt(stream, v, spaces+1);
				}
				sep = ',';
			}
			if (sep == '[') stream << sep;
			stream << ']';
		}break;
		case json::object: {
			if (json.empty()) stream << "{ }";
			else{
				char sep = ' ';
				stream << "{";
				bool sepln = false;
				for (Value v : json) {
					stream << sep; sep = ',';
					putSpaces(stream,spaces+1);
					stream << v.getKey() << ": ";
					simpleFmt(stream, v, spaces+2);
				}
				putSpaces(stream,spaces);
				stream << "}";
			}
		}break;
	}
}


class RpcClient: public quark::RpcClient {
public:
	virtual void onNotify(const Notify &ntf) override {
		std::cout<<"NOTIFY: " << ntf.eventName << " " ;
		if (jsonOut) ntf.data.toStream(std::cout);
		else simpleFmt(std::cout,ntf.data,0);
		std::cout<<std::endl;
	}
	virtual void logError(const Value &error) override {
		std::cout<<"ERROR: ";
		if (error.empty()) {
			std::cout << error.toString() << std::endl;
		} else {
			for (Value v: error) {
				std::cout << v.toString() << std::endl << "\t";
			}
			std::cout << std::endl;
		}

	}
	void prompt() {
		if (isConnected()) {
			std::cout << getAddr() << "> ";
			std::cout.flush();
		} else {
			std::cout << "...disconnected...> ";
			std::cout.flush();
		}
	}
	void setJsonOutput(bool jsonOut) {this->jsonOut = jsonOut;}
	bool jsonOut = false;
};


template<typename Fn>
class JsonParser: public json::Parser<Fn> {
public:
	JsonParser(const Fn &source) :json::Parser<Fn>(source) {}

	virtual Value parse() {
		char c = this->rd.nextWs();
		if (c == '{') {
			this->rd.commit(); return parseObject();
		} else {
			return json::Parser<Fn>::parse();
		}
	}

	inline typename Parser<Fn>::StrIdx readField()
	{
		std::size_t start = this->tmpstr.length();
		int c = this->rd.next();
		while (isalnum(c)) {
			this->rd.commit();
			this->tmpstr.push_back((char)c);
			c = this->rd.next();
		}
		return typename Parser<Fn>::StrIdx(start, this->tmpstr.size()-start);

	}


	inline Value parseObject()
	{
		std::size_t tmpArrPos = this->tmpArr.size();
		int c = this->rd.nextWs();
		if (c == '}') {
			this->rd.commit();
			return Value(object);
		}
		typename Parser<Fn>::StrIdx name(0,0);
		bool cont;
		do {
			if (c == '"') {
				this->rd.commit();
				try {
					name = this->readString();
				}
				catch (ParseError &e) {
					e.addContext(Parser<Fn>::getString(name));
					throw;
				}
			} else {
				name = readField();
			}
			try {
				int x = this->rd.nextWs();
				if (x != ':')
					throw ParseError("Expected ':'", x);
				this->rd.commit();
				Value v = parse();
				this->tmpArr.push_back(Value(Parser<Fn>::getString(name),v));
				Parser<Fn>::freeString(name);
			}
			catch (ParseError &e) {
				e.addContext(Parser<Fn>::getString(name));
				Parser<Fn>::freeString(name);
				throw;
			}
			c = this->rd.nextWs();
			this->rd.commit();
			if (c == '}') {
				cont = false;
			}
			else if (c == ',') {
				cont = true;
				c = this->rd.nextWs();
			}
			else {
				throw ParseError("Expected ',' or '}'", c);
			}
		} while (cont);
		StringView<Value> data = this->tmpArr;
		Value res(object, data.substr(tmpArrPos));
		this->tmpArr.resize(tmpArrPos);
		return res;
	}

};


void skipToNl(std::istream &in) {
	int c = in.get();
	while (c != EOF && c != '\n') c = in.get();
}

int main(int argc, char **argv) {



	if (argc == 1) {
		std::cerr << "Usage: " << argv[0] << " <address>:port" << std::endl;
		return 1;
	}

	maxPrecisionDigits=9;

	String addr = argv[1];

	RpcClient client;


	if (!client.connect(addr)) {
		std::cerr << "Failed to connect: " << addr << std::endl;
		return 2;
	}




	String prompt;

	std::string methodName;
	client.prompt();

	int c = std::cin.get();
	bool jsonout = false;

	while (c != EOF) {

		if (!isspace(c)) {

			methodName.clear();

			while (c != '[' && c != '{' && !isspace(c) && c!='\r' && c != '\n') {
				methodName.push_back((char)c);
				c = std::cin.get();
			}
			std::cin.putback(c);

			if (methodName.empty()) {
				std::cerr << "ERROR: requires name of method" << std::endl;
				skipToNl(std::cin);
			} else {

				if (methodName == "/quit" || methodName == "/q")
						break;
				else if (methodName == "/disconnect" || methodName == "/d")
						client.disconnect(true);
				else if (methodName == "/reconnect" || methodName == "/r") {
						client.disconnect(true);
						client.connect(client.getAddr());
				}
				else if (methodName == "/json") client.setJsonOutput(jsonout = true);
				else if (methodName == "/text") client.setJsonOutput(jsonout = false);
				else if (methodName == "/connect" || methodName == "/c") {
						std::string ln;
						c = std::cin.get();
						while (isspace(c)) c = std::cin.get();;
						std::cin.putback(c);
						std::getline(std::cin, ln);
						client.disconnect(true);
						client.connect(ln);
						client.prompt();
				} else  if (methodName == "/help" || methodName == "/h") {

					std::cout << "Usage:" << std::endl
							<< std::endl
							<< "function[]       - call 'function' without arguments" << std::endl
							<< "function[<args>] - call 'function' with arguments. Replace <args> with comma separated arguments" << std::endl
							<< "function{<obj>}  - call 'function' with named arguments. Replace <obj> with comma separated key:value argumets" << std::endl
							<< std::endl
							<< "/quit /q         - quit the program" << std::endl
							<< "/disconnect /d   - disconnect but stay standby" << std::endl
							<< "/reconnect  /r   - reconnect (reinitialize connection)" << std::endl
							<< "/json            - output result as json" << std::endl
							<< "/text	         - output result as formatted text" << std::endl
							<< "/help /h         - this help" << std::endl;

				} else

				try {
					JsonParser<json::StreamFromStdStream> parser((json::StreamFromStdStream(std::cin)));
					Value args = parser.parse();
					String name((StrViewA(methodName)));

					RpcResult res = client(name, args);
					if (res.isError()) {
						std::cerr << "ERROR: ";
						simpleFmt(std::cerr, res);
						std::cerr << std::endl;
					} else {
						if (jsonout)
							std::cout << res.toString() << std::endl;
						else {
							simpleFmt(std::cout, res);
							std::cout << std::endl;
						}
					}
				} catch (std::exception &e) {
					skipToNl(std::cin);
					std::cerr << "ERROR: " << e.what() << std::endl;
					std::cout << prompt<< std::flush;
				}
			}
		} else {
			if (c == '\n') {
				client.prompt();
			}
		}
		c = std::cin.get();
	}

	std::cout << std::endl;


}
