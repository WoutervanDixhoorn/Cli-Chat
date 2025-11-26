#include "server.h"

#include "msock.h"

#include <stdio.h>

#define SERVER_RECEIVE_BUFFER_SIZE 1024
static char receive_buffer[SERVER_RECEIVE_BUFFER_SIZE];

bool handle_connect(msock_client *client)
{
    printf("Client with ip: %s has connected\n", client->ip_addr);

    return true;
}

bool handle_client(msock_server *server, msock_client *client)
{
    (void)server;

    msock_message result_msg = {
        .buffer = receive_buffer,
        .size = SERVER_RECEIVE_BUFFER_SIZE
    };

    if(!msock_client_receive(client, &result_msg)) {
        printf("Client receive failed!\n");
        return false;
    }

    printf("Received message from client: %s\n", receive_buffer);

    if(!msock_server_broadcast(server, &result_msg)) {
        printf("Client send failed!\n");
        return false;
    }

    return true;
}

void chat_server_run()
{
    printf("Run chat server!\n");

    msock_server server;
    msock_server_listen(&server, "127.0.0.1", "420");

    msock_server_set_connect_cb(&server, handle_connect);
    msock_server_set_client_cb(&server, handle_client);
    
    while(msock_server_is_listening(&server)) {
        if(!msock_server_run(&server)) break;
    }

    msock_server_close(&server);
}