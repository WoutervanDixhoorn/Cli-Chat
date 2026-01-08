#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include <stdbool.h>

#include "../msock.h"
#include "../mconsole.h"

#define CLIENT_BUFFER_SIZE 256

#define IP_MAX_LEN 64
#define PORT_MAX_LEN 16

#define USERNAME_MAX_LEN 64

#define INPUT_MAX_LEN 256
#define HISTORY_MAX_LEN 4096

typedef enum {
    STATE_LOGIN,
    STATE_CONNECTING,
    STATE_CHAT,
    STATE_EXIT
} client_state;

typedef struct {
    char ip_buf[IP_MAX_LEN];
    char port_buf[PORT_MAX_LEN];
    char username_buf[USERNAME_MAX_LEN];
} login_state_data;

typedef struct {
    char history_buf[HISTORY_MAX_LEN];
    size_t history_len;
} chat_state_data;

typedef struct {
    client_state state;
    bool running;

    mconsole_context console_ctx;
    msock_client socket;

    uintptr_t h_receive_thread;

    char final_username_buf[USERNAME_MAX_LEN];

    union {
        login_state_data login;
        chat_state_data chat;
    } data;
    
} client_context;

typedef struct {
    char username[USERNAME_MAX_LEN];
} client_user_data;

bool chat_client_init(client_context *ctx);
void chat_client_deinit(client_context *ctx);

void state_to_login(client_context *ctx);
void state_to_connecting(client_context *ctx, char* username, char* ip_address, char* port);
void state_to_chat(client_context *ctx);

void tick_connecting(client_context *ctx);
void tick_chat(client_context *ctx);

bool chat_client_shouldclose(client_context *ctx);

void chat_client_run();

#endif // CHAT_CLIENT_H