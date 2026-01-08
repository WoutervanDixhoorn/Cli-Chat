#include "state_login.h"

static login_context state_context = {0};

bool state_login_init()
{
    state_context.active_field = 0;
    return true;
}

int next_field = 0;
void state_login_tick(client_context *c_ctx)
{
    if(state_context.next_state) {
        state_context.next_state = false;
        state_to_connecting(c_ctx, state_context.username, state_context.ip_address, state_context.port);
    }
    
    state_context.active_field = next_field;
}

void state_login_render(mconsole_context *mc_ctx)
{
    mconsole_begin(mc_ctx);
    
    mconsole_rect screen = mconsole_screen_rect(mc_ctx);
    mconsole_rect win = mconsole_rect_centered(screen, 50, 14);
    mconsole_draw_border(mc_ctx, win);

    mconsole_rect content = mconsole_rect_centered(win, 40, 6); 

    mconsole_ui_spacer(&content, 1); 

    if(mconsole_ui_labeld_input(mc_ctx, &content, "Username:", state_context.username, sizeof(state_context.username), state_context.active_field == 0)) {
        next_field = 1; 
    }
    
    if(mconsole_ui_labeld_input(mc_ctx, &content, "Ip Addr:", state_context.ip_address, sizeof(state_context.ip_address), state_context.active_field == 1)) {
        next_field = 2;
    }

    if(mconsole_ui_labeld_input(mc_ctx, &content, "Port:", state_context.port, sizeof(state_context.port), state_context.active_field == 2)) {
        state_context.next_state = true;
        next_field = 3;
    }

    mconsole_draw_debug_overlay(mc_ctx);
    mconsole_end(mc_ctx);
}