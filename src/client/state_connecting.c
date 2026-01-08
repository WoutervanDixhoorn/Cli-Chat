#include "state_connecting.h"

static bool connected = false;

bool state_connecting_init(client_context *c_ctx, char* username, char* ip_address, char* port)
{
    mconsole_debug_log("Attempting to connect to IP: '%s' Port: '%s'...\n", ip_address, port);

    msock_client_create(&c_ctx->socket);
    if(!msock_client_connect(&c_ctx->socket, ip_address, port)) {
        mconsole_debug_log("Conn Failed! Error: %d", WSAGetLastError());
        return false;
    }

    mconsole_debug_log("Connection Success!\n");

    msock_message username_msg = {
        .buffer = username,
        .len = strlen(username)
    };

    if(!msock_client_send(&c_ctx->socket, &username_msg)) return false;

    memcpy(&c_ctx->final_username_buf, username, strlen(username));
    
    return true;
}

void state_connecting_tick(client_context *c_ctx)
{
    if(connected) return;

    char buffer[256];
    msock_message msg = { .buffer = buffer, .size = sizeof(buffer) };

    int bytes = msock_client_receive(&c_ctx->socket, &msg);

    if (bytes > 0) {
        if (strncmp(buffer, "welcome", 7) == 0) {
            connected = true;
            mconsole_debug_log("User login Success!\n");
            state_to_chat(c_ctx);
        }
    } 
    else if (bytes == 0) {
        mconsole_debug_log("User login failed!\n");
        state_to_login(c_ctx);
        msock_client_close(&c_ctx->socket);
    }
    else if (bytes == -1) {
        return;
    }
}

void state_connecting_render(mconsole_context *mc_ctx)
{
    mconsole_begin(mc_ctx);
    mconsole_rect screen = mconsole_screen_rect(mc_ctx);
    mconsole_text_box(mc_ctx, screen, "Connecting to server...", false, MCONSOLE_ALIGN_LEFT);

    mconsole_draw_debug_overlay(mc_ctx);

    mconsole_end(mc_ctx);
}