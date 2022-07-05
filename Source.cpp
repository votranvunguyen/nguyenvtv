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
#include"Poco/StreamCopier.h"
using Poco::StringTokenizer;
using Poco::cat;



using namespace Poco::Net;
using namespace Poco::Util;
using namespace std;


void addPerson(tutorial::Person* person) {
	cout << "old: ";
	int old;
	cin >> old;
	person->set_old(old);
	cout << " has license YES / NO " << endl ;
	string has;
	cin >> has;
	if (has == "yes")
	{
		person->set_has_license(true);
	}
	else
	{
		person->set_has_license(false);
	}

	cout << "name: " ;
	string name;
	cin>> name;
	if (!name.empty())
	{
		person->set_name(name);
	}
	
	while (true)
	{
		cout << " enter name children (or 00 finish): " ;
		string name_children;
		cin>> name_children;
		if (name_children=="00") {
			break;
		}
		tutorial::Person::CHILDREN* children_number = person->add_children();
		children_number->set_nb_children(name_children.c_str());
	}
	
}
void addDataHouse(tutorial::House* house, tutorial::Person* ps) {	
	cout << " HOUSE_TYPE wook / cement : " << endl;
	string h_t;
	cin >> h_t;
	if (h_t == "wook")
	{
		house->set_type(tutorial::House::HOUSE_TYPE::House_HOUSE_TYPE_WOOD);
	}
	else
	{
		house->set_type(tutorial::House::HOUSE_TYPE::House_HOUSE_TYPE_CEMENT);
	}

	int nb_of_lf;
	cout << "nb_of_lf: " << endl;
	cin >> nb_of_lf;
	house->set_number_of_floors(nb_of_lf);

	house->set_allocated_owner(ps);
}

void addDataCar(tutorial::Car* car,tutorial::Person* ps) {
	car->set_allocated_owner(ps);
	string   type;

	cout << " car_type  OIL / GAS / HYBRID : " << endl;
	cin >> type;

	if (type == "gas")
	{
		car->set_type(tutorial::Car::CAR_TYPE::Car_CAR_TYPE_GAS);
	}
	if (type == "hybrid")
	{
		car->set_type(tutorial::Car::CAR_TYPE::Car_CAR_TYPE_HYBRID);
	}
	else
	{
		car->set_type(tutorial::Car::CAR_TYPE::Car_CAR_TYPE_OIL);
	}


	int numb;
	cout << "number_of_wheel: " << endl;
	cin >> numb;
	car->set_number_of_wheel(numb);

}

void addDatasend(tutorial::DataToSend* data, tutorial::House* h, tutorial::Car* car)
{
	data->set_allocated_house(h);
	data->set_allocated_car(car);
	
}

void listData(tutorial::DataToSend& data) {
	cout << "----------------------HOUSE----------------" << endl;
	if (data.house().type() == tutorial::House::HOUSE_TYPE::House_HOUSE_TYPE_WOOD) {
		cout << "WOOD" << endl;

	}
	else
	{
		cout << "CEMENT" << endl;
	}

	int old= data.house().owner().old();

	if (data.house().owner().has_license()==true)
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

	if (data.car().type()==tutorial::Car::CAR_TYPE::Car_CAR_TYPE_GAS)
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

void main() {
	try
	{
		GOOGLE_PROTOBUF_VERIFY_VERSION;
		string uri = "10.225.0.230";
		int port = 9090;
		HTTPClientSession sv(uri, port);
		tutorial::Person* ps = new tutorial::Person;
		tutorial::House* house = new tutorial::House;
		tutorial::Car* car= new tutorial::Car;
		tutorial::DataToSend* data = new tutorial::DataToSend;
	//	data.mutable_house();
	//	data.mutable_car();
	//	delete house;
		string strData;
		HTTPRequest request(HTTPRequest::HTTP_POST, "/admin");
		addPerson(ps);	
		addDataHouse(house,ps);
		addDataCar(car, ps);	
		addDatasend(data, house, car);
		listData(*data);	
		tutorial::DataToSend s = *data;
		s.SerializeToString(&strData);	
		sv.sendRequest(request)<<strData;
		HTTPResponse response;
		std::istream& rs = sv.receiveResponse(response);
		Poco::StreamCopier::copyStream(rs, std::cout);
		data->release_car();
		data->release_house();
		car->release_owner();
		house->release_owner();
		delete ps;
		delete car;
		delete house;
		delete data;
		
	}
	catch (Poco::Exception& e)
	{
		cout << e.displayText() << endl;
	}
	google::protobuf::ShutdownProtobufLibrary();
	
}
