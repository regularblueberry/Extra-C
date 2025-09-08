#pragma once
#include "../test-utils.h"

inst(Mutex) network_mutex = NULL;

#define SERVER_ARG_RUN_LOCAL_TEST (void*)1
#define SERVER_ARG_RUN_IPV4_TEST (void*)2
#define SERVER_ARG_RUN_IPV6_TEST (void*)3

int server_func(inst(Thread) thread, void* args){

	switch (addrasval(args)) {
	case 1:{


	break;}	
	case 2:{


	break;}	
	case 3:{


	break;}	
	}
	inst(Socket) test_socket = new(Socket,
		(socket_settings){
			.blocking = false,
			.domain = SOCKET_DOMAIN_LOCAL,
			.protocol = SOCKET_PROTOCOL_RAW
		});
	
	Socket.Bind(test_socket, &(address_socklocal){s("/tmp/__TEST_SOCKET")});

	Socket.Listen(test_socket, 1);

	inst(Connection) client = Socket.Accept(test_socket);
	char hi[13] = "Hello, World";

	Connection.Send(client, msgBuff(hi, 12));

	del(test_socket);
	del(client);
return 0;
}


bool RUN_NETWORK_TESTS(){
	inst(Thread) server_thread = new(Thread, server_func);
	network_mutex = new(Mutex);
	

NEW_TEST("Socket Connection Local"){
	
	Thread.Start(server_thread, SERVER_ARG_RUN_LOCAL_TEST);
	
	inst(Connection) test_sever = new(Connection,
		(socket_settings){
			.blocking = false,
			.domain = SOCKET_DOMAIN_LOCAL,
			.protocol = SOCKET_PROTOCOL_RAW
		},
		&(address_socklocal){s("/tmp/__TEST_SOCKET")});

	char msg[13] = {0};
	Connection.Recieve(test_sever, msgBuff(msg, 12));

	del(test_sever);
}

NEW_TEST("Socket Connection IPV4"){}
NEW_TEST("Socket Connection IPV6"){}

NEW_TEST("Domain Name Resolution"){







}
return TEST_RESULT;
}
