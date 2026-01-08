#ifndef STATE_LOGIN_H
#define STATE_LOGIN_H

#include "client.h"

typedef struct {
    char username[64];
    char ip_address[64];
    char port[32];

    int active_field; // 0 = Username, 1 = IP
    bool next_state;
} login_context;

bool state_login_init();
void state_login_tick(client_context *c_ctx);
void state_login_render(mconsole_context *mc_ctx);

#endif