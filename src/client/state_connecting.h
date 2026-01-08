#ifndef STATE_CONNECTING_H
#define STATE_CONNECTING_H

#include "client.h"

bool state_connecting_init(client_context *c_ctx, char* username, char* ip_address, char* port);
void state_connecting_tick(client_context *c_context);
void state_connecting_render(mconsole_context *mc_ctx);

#endif