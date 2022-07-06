#pragma once
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Util/ServerApplication.h>
#include <iostream>
#include <string>
#include"Poco/Net/HTTPClientSession.h"
#include"Poco/StreamCopier.h"

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>


using namespace Poco::Net;
using namespace Poco::Util;
using namespace std;

void sendStringRequest_POST(string req, string data, HTTPClientSession& s) {

	HTTPRequest Request(HTTPRequest::HTTP_POST, req);
	s.sendRequest(Request) << data;
	cout << "send..." << endl;

	/*std::ostream& ostr = sv.sendRequest(request);
	s.SerializeToOstream(&ostr);*/
}

void getDataServerToClient(HTTPClientSession& s) {

	HTTPResponse response;
	std::istream& rs = s.receiveResponse(response);
	Poco::StreamCopier::copyStream(rs, std::cout);
}

void serverSendData(HTTPServerResponse& resp,string dataSend) {

	auto& out = resp.send();
	out << dataSend;
	out.flush();

}

string getDataClientToServer(HTTPServerRequest& req) {

	string strdata;
	Poco::StreamCopier::copyToString(req.stream(), strdata);
	return strdata;

}
