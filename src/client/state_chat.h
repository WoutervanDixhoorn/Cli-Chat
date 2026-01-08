#ifndef STATE_chat_H
#define STATE_chat_H

#include "client.h"

#ifdef _WIN32
    #include <process.h>
#else
    #include <pthread.h>
#endif

#define CLICHAT_HISTORY_SIZE 2048

typedef struct {
    char* username;

    char input_buffer[256];
    bool send_requested;
    
    char message_buffer[CLICHAT_HISTORY_SIZE];
    size_t current_message_index;

#ifdef _WIN32
    CRITICAL_SECTION lock;
#else
    pthread_mutex_t lock;
#endif
} chat_context;

bool state_chat_init(client_context * c_ctx, char* username);
void state_chat_tick(client_context *c_ctx);
void state_chat_render(mconsole_context *mc_ctx);

void add_message(char *msg);
bool handle_client_send(msock_client *client, char *message);
void lock_chat_state();
void unlock_chat_state();

#endif