#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include"iostream"
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <assert.h>
#include<stdio.h>
#include <winsock2.h>
#include"person_house_car.pb.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>


using namespace std;

void ConnectionRabbitMQ(amqp_connection_state_t& connect,amqp_socket_t* socket,
	std::string hostname,int port,std::string user,std::string password ,std::string vhost ) {

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

void bindQueue(amqp_connection_state_t connection,string  queue,std::string bindkey,std::string exchang) {
	 amqp_queue_bind_ok_t* r =
        amqp_queue_bind(connection, 1, amqp_cstring_bytes(queue.c_str()),
                        amqp_cstring_bytes(exchang.c_str()),
                        amqp_cstring_bytes(bindkey.c_str()), amqp_empty_table);
	 amqp_get_rpc_reply(connection), "Consuming";
}

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

void getMessage(amqp_connection_state_t conn,string queue,string bindkey) {

	for (;;) {
		amqp_rpc_reply_t res;
		amqp_envelope_t env;

		amqp_maybe_release_buffers(conn);
		amqp_basic_consume_ok_t* r =
			amqp_basic_consume(conn, 1, amqp_cstring_bytes(queue.c_str()), amqp_cstring_bytes(bindkey.c_str()), 0, 0, 0, amqp_empty_table);
	
		res = amqp_consume_message(conn, &env, NULL, 0);
		
		if (AMQP_RESPONSE_NORMAL != res.reply_type) {
			break;
		}
		string textbytes((char*)env.message.body.bytes, env.message.body.len);
		tutorial::DataToSend* data = nullptr;
		data=new tutorial::DataToSend;
		if(data->ParseFromString(textbytes)){
		listData(*data);
		amqp_destroy_envelope(&env);	
		if(data!=nullptr)
		delete data;
		break;

		}
	}

}
void disConnect(amqp_connection_state_t conn) {
	amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
	amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
	amqp_destroy_connection(conn);
	
}

int main() {
	amqp_socket_t* socket = NULL;
	amqp_connection_state_t connection;
	string queuename="test_rabbit";

	int port = 5672;
	string exchange = "amq.topic";
	string bindkey = "red.green.*";
	string hosts = "localhost";
	string user = "guest";
	string password = "guest";
	string vhost = "/";
	

	ConnectionRabbitMQ(connection, socket, hosts, port, user, password, vhost);
	bindQueue(connection, queuename,bindkey,exchange);
	getMessage(connection,queuename,bindkey);
	disConnect(connection);
	google::protobuf::ShutdownProtobufLibrary();
	_CrtDumpMemoryLeaks();
	system("pause");
	return 0;
}