#include "client.h"

#include "msock.h"

#include <stdio.h>
#include <process.h>

#define CLIENT_BUFFER_SIZE 1024

void client_receive_thread(void *arg)
{
    msock_client *client = (msock_client*)arg;

    char receive_buffer[CLIENT_BUFFER_SIZE];
    msock_message result_msg = {
        .buffer = receive_buffer,
        .size = sizeof(receive_buffer)
    };

    while(msock_client_is_connected(client)) {

        if(!msock_client_receive(client, &result_msg)) {
            printf("msock_client_receive() failed!\n");
            break;
        }
        
        printf("\r%s\n> ", result_msg.buffer);
    }

    msock_client_close(client);
}

bool handle_client_send(msock_client *client)
{
    char input[CLIENT_BUFFER_SIZE];

    printf("> ");

    if(fgets(input, CLIENT_BUFFER_SIZE, stdin) == NULL) return false;

    input[strcspn(input, "\n")] = 0;

    if (strlen(input) == 0) return true;

    msock_message send_msg = {
        .buffer = input,
        .len = strlen(input)
    };

    return msock_client_send(client, &send_msg); 
}

bool client_user_handshake(msock_client *client, char *username)
{
    msock_message username_msg = {
        .buffer = username,
        .len = strlen(username)
    };

    if(!msock_client_send(client, &username_msg)) return false;

    char success[CLIENT_BUFFER_SIZE];
    msock_message success_msg = {
        .buffer = success,
        .size = sizeof(success)
    };

    if(!msock_client_receive(client, &success_msg)) return false;
    
    if(strcmp(success, "welcome") != 0) return false;

    printf("Handshake was successful!\n");

    return true;
}

void chat_client_run() 
{
    printf("Run chat client!\n");

    msock_client client;
    msock_client_create(&client);
    msock_client_connect(&client, "127.0.0.1", "420");

    char username[CLIENT_BUFFER_SIZE];
    printf("Whats your username: ");
    if(fgets(username, CLIENT_BUFFER_SIZE, stdin) == NULL) goto defer;
    username[strcspn(username, "\n")] = 0;
    if (strlen(username) == 0) {
        printf("Username should be atleast one character\n");
        goto defer;
    }

    //Do user handshake
    if(!client_user_handshake(&client, username)) goto defer;

    //Begin thread receive
    _beginthreadex(NULL, 0, (void*)client_receive_thread, &client, 0, NULL);

    while(msock_client_is_connected(&client)) {
        if(!handle_client_send(&client)) break;
    }

defer:    
    msock_client_close(&client);
}