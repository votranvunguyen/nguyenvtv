
#include"person_house_car.pb.h"

#include"Poco/JSON/Object.h"
#include"Poco/JSONString.h"
#include"Poco/JSON/Parser.h"
#include <fstream>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Util/ServerApplication.h>
#include <iostream>
#include <string>
#include <vector>
#include"Poco/URI.h"
#include"Poco/Net/HTTPClientSession.h"
#include"Poco/StreamCopier.h"
#include "Poco/StringTokenizer.h"
#include "Poco/String.h"
using namespace std;

using Poco::StringTokenizer;
using Poco::cat;



using namespace Poco::Net;
using namespace Poco::Util;

class MyRequestHandler:public HTTPRequestHandler
{
public:
	virtual void handleRequest(HTTPServerRequest& req,HTTPServerResponse& resp) {
		
		if (req.getURI() == "/admin") {
			tutorial::DataToSend data;
			string strdata;
			Poco::StreamCopier::copyToString(req.stream(), strdata);
			data.ParseFromString(strdata);
			listData(data);

			auto& out1 = resp.send();
			out1 <<"server da nhan !!!";

			out1.flush();

			google::protobuf::ShutdownProtobufLibrary();
		}


	}
private:
	void listData(tutorial::DataToSend& data)
	{
		cout << "----------------------HOUSE----------------" << endl;
		if (data.house().type() == tutorial::House::HOUSE_TYPE::House_HOUSE_TYPE_WOOD)
		{
			cout << "WOOD" << endl;
		}
		else
		{
			cout << "CEMENT" << endl;
		}

		int old = data.house().owner().old();

		if (data.house().owner().has_license() == true)
		{
			cout << "co giay phep !!" << endl;
		}
		else
		{
			cout << "khong co giay phep !!" << endl;
		}
		string name = data.house().owner().name();

		for (int i = 0; i < data.house().owner().children_size(); i++)
		{
			cout << data.house().owner().children(i).nb_children() << endl;
		}

		int num_of_floo = data.house().number_of_floors();

		cout << "old: " << old << endl;
		cout << "name: " << name << endl;
		cout << "number_of_floors: " << num_of_floo << endl;

		cout << "------------------------CAR-------------------------" << endl;
		int odl_ca = data.car().owner().old();

		if (data.car().owner().has_license() == true)
		{
			cout << "co giay phep !!" << endl;
		}
		else
		{
			cout << "khong co giay phep !!" << endl;
		}
		string name_car = data.car().owner().name();

		for (int i = 0; i < data.car().owner().children_size(); i++)
		{
			cout << data.car().owner().children(i).nb_children() << endl;
		}

		if (data.car().type() == tutorial::Car::CAR_TYPE::Car_CAR_TYPE_GAS)
		{
			cout << "GAS" << endl;

		}
		else if (data.car().type() == tutorial::Car::CAR_TYPE::Car_CAR_TYPE_HYBRID) {
			cout << "HYBRID" << endl;
		}
		else
		{
			cout << "OIL" << endl;

		}
		int num_of_wheel = data.car().number_of_wheel();

		cout << "old: " << odl_ca << endl;
		cout << "name: " << name_car << endl;
		cout << "number_of_floors: " << num_of_wheel << endl;

	}
};

class myRequestFactory:public HTTPRequestHandlerFactory
{
public:
	virtual HTTPRequestHandler* createRequestHandler(const HTTPServerRequest&) {
		return new MyRequestHandler;
	}
private:

};


class MyClass:public ServerApplication
{
protected:
	int main(const vector<string>&) {
		HTTPServer sv(new myRequestFactory,ServerSocket(9090),new HTTPServerParams);
		sv.start();
		cout << "SERVER START!!!" << endl;
		waitForTerminationRequest();
		
		cout << endl << "shut down..." << endl;
		sv.stop();

		return Application::EXIT_OK;
	}

private:

};

int main(int argc, char** argv) {
	MyClass myClass;
	myClass.run(argc,argv);



}