#include "client.h"

#include "msock.h"

#include <stdio.h>

#ifdef _WIN32
    #include <process.h>
#else
    #include <pthread.h>
#endif

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

        ssize_t bytes = msock_client_receive(client, &result_msg);
        if(bytes < 0) {
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

    if(!msock_client_send(client, &send_msg)) return false;

    printf("%s\n", input);

    return true;
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

    ssize_t bytes = msock_client_receive(client, &success_msg);
    if(bytes < 0) return false;
    
    if(strcmp(success, "welcome") != 0) return false;

    printf("Handshake was successful!\n");

    return true;
}

void chat_client_run() 
{
    char server_addr[64];
    printf("What's the IP of the chat server: ");
    if(fgets(server_addr, sizeof(server_addr), stdin) == NULL) return;
    server_addr[strcspn(server_addr, "\n")] = 0;

    char server_port[64];
    printf("What's the port of the chat server: ");
    if(fgets(server_port, sizeof(server_port), stdin) == NULL) return;
    server_port[strcspn(server_port, "\n")] = 0;

    printf("Trying to connect to %s:%s\n", server_addr, server_port);

    msock_client client = {0};
    msock_client_create(&client);
    if(!msock_client_connect(&client, server_addr, server_port)) goto defer;

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
    
#ifdef _WIN32
    _beginthreadex(NULL, 0, (void*)client_receive_thread, &client, 0, NULL);
#else
    pthread_t receive_thread;
    pthread_create(&receive_thread, NULL, (void*)client_receive_thread, (void*)&client);
#endif

    while(msock_client_is_connected(&client)) {
        if(!handle_client_send(&client)) break;
    }

defer:    

#ifndef _WIN32
    pthread_join(receive_thread, NULL);
#endif

    msock_client_close(&client);
}