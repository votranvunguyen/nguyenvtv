#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include"iostream"
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <assert.h>
#include<WinSock2.h>
#include"person_house_car.pb.h"

#define _CRTDBG_MAP_ALLOC


#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif  // _DEBUG



using namespace std;

void ConnectionRabbitMQ(amqp_connection_state_t& connect, amqp_socket_t* socket,
    std::string hostname, int port, std::string user, std::string password, std::string vhost) {

    connect = amqp_new_connection();
    socket = amqp_tcp_socket_new(connect);
    if (!socket)
    {
        cout << "creating TCP socket" << endl;
    }
    int status = amqp_socket_open(socket, hostname.c_str(), port);
    if (status)
    {
        cout << "open TCP socket" << endl;
    }

    amqp_login(connect, vhost.c_str(), 60, 131072, 0, AMQP_SASL_METHOD_PLAIN, user.c_str(), password.c_str());
    amqp_channel_open(connect, 1);
    amqp_get_rpc_reply(connect);
}

void addPerson(tutorial::Person* person) {
	cout << "old: ";
	int old;
	cin >> old;
	person->set_old(old);
	cout << " has license YES / NO " << endl;
	string hasLicense;
	cin >> hasLicense;
	if (hasLicense == "yes")
	{
		person->set_has_license(true);
	}
	else
	{
		person->set_has_license(false);
	}

	cout << "name: ";
	string name;
	cin >> name;
	if (!name.empty())
	{
		person->set_name(name);
	}

	while (true)
	{
		cout << " enter name children (or 00 finish): ";
		string name_children;
		cin >> name_children;
		if (name_children == "00") {
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

void addDataCar(tutorial::Car* car, tutorial::Person* ps) {
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

bool sendMessage(amqp_connection_state_t conn, string exchange, string routingkey, amqp_bytes_t queue, string message) {
    amqp_basic_properties_t props;
    props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG |
        AMQP_BASIC_DELIVERY_MODE_FLAG | AMQP_BASIC_REPLY_TO_FLAG |
        AMQP_BASIC_CORRELATION_ID_FLAG;
    props.content_type = amqp_cstring_bytes("text/plain");
    props.delivery_mode = 2; /* persistent delivery mode */
    props.reply_to = amqp_bytes_malloc_dup(queue);
   
    if (props.reply_to.bytes == NULL) {
        fprintf(stderr, "Out of memory");
        return false;
    }
    props.correlation_id = amqp_cstring_bytes("1");
    amqp_basic_publish(conn, 1, amqp_cstring_bytes(exchange.c_str()),
        amqp_cstring_bytes(routingkey.c_str()), 0, 0,
        &props, amqp_cstring_bytes(message.c_str()));
    amqp_bytes_free(props.reply_to);

    return true;
}
void create_queue_client(amqp_connection_state_t conn, amqp_bytes_t &reply_to_queue)
{
    amqp_queue_declare_ok_t* r = amqp_queue_declare(
        conn, 1, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);
	amqp_get_rpc_reply(conn);
    reply_to_queue = amqp_bytes_malloc_dup(r->queue);
    
    if (reply_to_queue.bytes == NULL) {
       cout<< "Out of memory while copying queue name";
        return ;
    }
   
}
int main(int argc, char* argv[]) {
    amqp_socket_t* socket = NULL;
    amqp_connection_state_t conn;
    amqp_bytes_t reply_to_queue;
    string hostname = "localhost";
    int port = 5672;
    string exchange = "amq.topic";
    string routingkey = "red.green.blue";
    string messagebody = "hello wordl!!!_www";
    string user = "guest";
    string password = "guest";
    string vhost = "/";  
	tutorial::Person* ps = new tutorial::Person;

	tutorial::DataToSend* data = new tutorial::DataToSend;
	tutorial::House* house = data->mutable_house();
	tutorial::Car* car = data->mutable_car();

	string strData;
	
	addPerson(ps);
	addDataHouse(house, ps);
	tutorial::Person* psClone = new tutorial::Person(*ps);
	addDataCar(car, psClone);
	//addDatasend(data, house, car);		
	data->SerializeToString(&strData);
	
      try
        {
          ConnectionRabbitMQ(conn, socket, hostname, port, user, password, vhost);
          create_queue_client(conn, reply_to_queue);
          if (sendMessage(conn, exchange, routingkey, reply_to_queue, strData)) {
              cout << "send........" << endl;
          }
          else {
              cout << "can not send..." << endl;
          }
        amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
        amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(conn);
   
        }
        catch (const std::exception& e)
        {
            std::cout << e.what()<<std::endl;
        }
		delete data;
		google::protobuf::ShutdownProtobufLibrary();
		_CrtDumpMemoryLeaks();
    system("pause");
    return 0;
}