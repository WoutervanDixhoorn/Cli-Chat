#include "server.h"

#include "msock.h"

#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>

#define SERVER_MAX_BUFFER_SIZE 1024

typedef struct {
    char username[64];
} client_data;

bool server_user_handshake(msock_client *client)
{
    char username_buffer[64];
    msock_message username_msg = {
        .buffer = username_buffer,
        .size = sizeof(username_buffer)
    };

    while(username_msg.len < 1){
        if(!msock_client_receive(client, &username_msg)) return false;
    }

    if(username_msg.len > 64) return false;

    char welcome[] = "welcome";
    msock_message welcome_msg = {
        .buffer = welcome,
        .len = strlen(welcome)
    };

    if(!msock_client_send(client, &welcome_msg)) return false;

    client_data *data = (client_data*)malloc(sizeof(client_data));
    memset(data, 0, sizeof(client_data));
    snprintf(data->username, sizeof(data->username), "%s", username_buffer);
    client->user_data = data;

    return true;
}

bool handle_connect(msock_client *client)
{
    printf("Client with ip: %s has connected\n", client->ip_addr);

    if(!server_user_handshake(client)) return false;

    client_data *data = (client_data*)client->user_data;
    printf("Handshake success with username: %s\n", data->username);

    return true;
}

bool handle_disconnect(msock_client *client)
{
    //Free user_data since is malloced
    free(client->user_data);

    return true;
}

bool handle_client(msock_server *server, msock_client *client)
{
    (void)server;

    char receive_buffer[SERVER_MAX_BUFFER_SIZE];
    msock_message result_msg = {
        .buffer = receive_buffer,
        .size = SERVER_MAX_BUFFER_SIZE
    };

    if(!msock_client_receive(client, &result_msg)) {
        printf("Client receive failed!\n");
        return false;
    }

    client_data *data = (client_data*)client->user_data;
    printf("%s: %s\n", data->username, receive_buffer);

    char return_buffer[SERVER_MAX_BUFFER_SIZE];
    int written_len = snprintf(return_buffer, sizeof(return_buffer), 
                               "%s: %s", data->username, receive_buffer);
    msock_message return_msg = {
        .buffer = return_buffer,
        .size   = sizeof(return_buffer), 
        .len    = written_len            
    };

    if(!msock_server_broadcast(server, &return_msg)) {
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
    msock_server_set_disconnect_cb(&server, handle_disconnect);
    msock_server_set_client_cb(&server, handle_client);
    
    while(msock_server_is_listening(&server)) {
        if(!msock_server_run(&server)) break;
    }

    msock_server_close(&server);
}