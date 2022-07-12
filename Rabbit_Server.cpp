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

void create_queue_server(amqp_connection_state_t conn,amqp_bytes_t& queue,std::string bindingkey,std::string exchang) {
	amqp_queue_declare_ok_t* red = amqp_queue_declare(conn, 1, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);
	amqp_get_rpc_reply(conn);
	queue = amqp_bytes_malloc_dup(red->queue);
	if (queue.bytes == NULL) {
		cout << "out of memory" << endl;
	}

	amqp_queue_bind(conn, 1, queue, amqp_cstring_bytes(exchang.c_str()),
		amqp_cstring_bytes(bindingkey.c_str()), amqp_empty_table);
	amqp_get_rpc_reply(conn);

	amqp_basic_consume(conn, 1, queue, amqp_empty_bytes, 0, 1, 0,
		amqp_empty_table);
	amqp_get_rpc_reply(conn), "Consuming";

}



bool sendMessage(amqp_connection_state_t conn,string exchange,string routingkey,amqp_bytes_t queue,string message) {
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

void getMessage(amqp_connection_state_t conn) {

	for (;;) {
		amqp_rpc_reply_t res;
		amqp_envelope_t env;

		amqp_maybe_release_buffers(conn);

		res = amqp_consume_message(conn, &env, NULL, 0);

		if (AMQP_RESPONSE_NORMAL != res.reply_type) {
			break;
		}
		string text_bytes((char*)env.message.body.bytes, env.message.body.len);
		tutorial::DataToSend* data=new tutorial::DataToSend;
		if(data->ParseFromString(text_bytes)){
		listData(*data);
		amqp_destroy_envelope(&env);	
		delete data;
		break;
		}
	}

}


void disConnect(amqp_connection_state_t conn, amqp_bytes_t queueName) {
	amqp_bytes_free(queueName);
	amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
	amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
	amqp_destroy_connection(conn);
	
}

int main() {
	amqp_socket_t* socket = NULL;
	amqp_connection_state_t conn;
	amqp_bytes_t queueName;

	int port = 5672;
	string exchange = "amq.topic";
	string bindingkey = "red.green.*";
	string hosts = "localhost";
	string user = "guest";
	string password = "guest";
	string vhost = "/";
	

	ConnectionRabbitMQ(conn, socket, hosts, port, user, password, vhost);
	create_queue_server(conn, queueName,bindingkey,exchange);
	getMessage(conn);
	disConnect(conn, queueName);
	google::protobuf::ShutdownProtobufLibrary();
	_CrtDumpMemoryLeaks();
	system("pause");
	return 0;
}