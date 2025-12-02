#include "client.h"
#include "msock.h"
#define MCONSOLE_IMPLEMENTATION
#include "mconsole.h"

#include <stdio.h>

#ifdef _WIN32
    #include <process.h>
#else
    #include <pthread.h>
#endif

static char client_username[64] = {0}; 

#define CLIENT_BUFFER_SIZE 1024

#define CLICHAT_HISTORY_SIZE 2048
char message_buffer[CLICHAT_HISTORY_SIZE] = {0};
size_t current_message_index = 0;

void add_message(char *msg) {
    size_t len = strlen(msg);
    if (current_message_index + len + 2 >= sizeo(message_buffer)) {
        current_message_index = 0;
        message_buffer[0] = '\0';
    }

    // Copy the new message to the END of the global buffer
    memcpy(&message_buffer[current_message_index], msg, len);
    
    current_message_index += len;
    message_buffer[current_message_index++] = '\n';
    message_buffer[current_message_index] = '\0';
}
void __stdcall client_receive_thread(void *arg)
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
        
        add_message(receive_buffer);
    }

    msock_client_close(client);
}

bool handle_client_send(msock_client *client, char *message)
{
    message[strcspn(message, "\n")] = 0;

    if (strlen(message) == 0) return true;

    msock_message send_msg = {
        .buffer = message,
        .len = strlen(message)
    };

    if(!msock_client_send(client, &send_msg)) return false;

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

    strcpy_s(client_username, sizeof(client_username), username);

    printf("Handshake was successful!\n");

    return true;
}

bool client_setup_network(msock_client *result_client) {
    char server_addr[64];
    printf("What's the IP of the chat server: ");
    if(fgets(server_addr, sizeof(server_addr), stdin) == NULL) return false;
    server_addr[strcspn(server_addr, "\n")] = 0;

    char server_port[64];
    printf("What's the port of the chat server: ");
    if(fgets(server_port, sizeof(server_port), stdin) == NULL) return false;
    server_port[strcspn(server_port, "\n")] = 0;

    printf("Trying to connect to %s:%s\n", server_addr, server_port);
    
    msock_client_create(result_client);
    if(!msock_client_connect(result_client, server_addr, server_port)) goto defer;

    char username[CLIENT_BUFFER_SIZE];
    printf("Whats your username: ");
    if(fgets(username, CLIENT_BUFFER_SIZE, stdin) == NULL) goto defer;
    username[strcspn(username, "\n")] = 0;
    if (strlen(username) == 0) {
        printf("Username should be atleast one character\n");
        goto defer;
    }

    //Do user handshake
    if(!client_user_handshake(result_client, username)) goto defer;

    return true;

defer:
    msock_client_close(result_client);
    return false;
}

char input_buffer[256];
void client_handle_ui(mconsole_context *ctx, msock_client *client) {
    mconsole_begin(ctx);

    mconsole_rect screen = mconsole_screen_rect(ctx);
    mconsole_rect input_rect = mconsole_cut_bottom(&screen, 3);
    mconsole_rect chat_rect = screen;

    mconsole_draw_border(ctx, input_rect);

    mconsole_rect input_text_area = mconsole_shrink(&input_rect, 1);
    if (mconsole_input_field(ctx, input_text_area, input_buffer, sizeof(input_buffer))) {
        if(handle_client_send(client, input_buffer)) {
            char msg[sizeof(input_buffer)+sizeof(client_username)];
            snprintf(msg, sizeof(msg), "%s: %s", client_username, input_buffer);
            add_message(msg);
        }
        memset(input_buffer, 0, sizeof(input_buffer));
    }

    mconsole_draw_border(ctx, chat_rect);
    mconsole_rect content = mconsole_shrink(&chat_rect, 1);
    mconsole_text_box(ctx, content, message_buffer, true, MCONSOLE_ALIGN_LEFT);

    mconsole_end(ctx);
}

void chat_client_run() 
{
    msock_client client = {0};
    if(!client_setup_network(&client)) return;

    mconsole_context ctx = {0};
    mconsole_init(&ctx);

    //Begin thread receive
#ifdef _WIN32
    _beginthreadex(NULL, 0, (void*)client_receive_thread, &client, 0, NULL);
#else
    pthread_t receive_thread;
    pthread_create(&receive_thread, NULL, (void*)client_receive_thread, (void*)&client);
#endif

    while(msock_client_is_connected(&client) && mconsole_running(&ctx)) {
        client_handle_ui(&ctx, &client);
        // if(!handle_client_send(&client)) break;
    }

#ifndef _WIN32
    pthread_join(receive_thread, NULL);
#endif

    msock_client_close(&client);
    mconsole_deinit(&ctx);
}