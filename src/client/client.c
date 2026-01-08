#include "client.h"

#include "state_login.h"
#include "state_connecting.h"
#include "state_chat.h"

void chat_client_run() 
{
    client_context ctx = {0};
    chat_client_init(&ctx);

    while(ctx.running && mconsole_running(&ctx.console_ctx)) {
        switch (ctx.state)
        {
        case STATE_LOGIN:
            state_login_tick(&ctx);
            state_login_render(&ctx.console_ctx);
            break;
        case STATE_CONNECTING:
            state_connecting_tick(&ctx);
            state_connecting_render(&ctx.console_ctx);
            break;
        case STATE_CHAT:
            state_chat_tick(&ctx);
            state_chat_render(&ctx.console_ctx);
            break;
        default:
            printf("UNREACHABLE!\n Something went horribly wrongg!!");
            break;
        }
    }
    
    chat_client_deinit(&ctx);
}

bool chat_client_init(client_context *ctx)
{
    if(!mconsole_init(&ctx->console_ctx)) {
        mconsole_debug_log("Failed mconsole_init()\n");
        return false;
    };

    ctx->running = true;
    state_to_login(ctx);
    return true;
}

void chat_client_deinit(client_context *ctx){
    (void)ctx;
}

void state_to_login(client_context *ctx)
{
    mconsole_debug_log("Transitioning to Login State...\n");

    if(state_login_init()) ctx->state = STATE_LOGIN;
}

void state_to_connecting(client_context *ctx, char* username, char* ip_address, char* port)
{
    mconsole_debug_log("Transitioning to Connecting State...\n");

    if(state_connecting_init(ctx, username, ip_address, port)) ctx->state = STATE_CONNECTING;
}

void state_to_chat(client_context *ctx) 
{   
    mconsole_debug_log("Transitioning to Chat State...\n");

    if(state_chat_init(ctx, ctx->final_username_buf)) ctx->state = STATE_CHAT;
}