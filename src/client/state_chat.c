#include "client.h"

#include "state_chat.h"

chat_context state_context = {0};

#ifdef _WIN32
unsigned __stdcall client_receive_thread(void* arg)
#else
void* client_receive_thread(void* arg)
#endif
{
    msock_client *client = (msock_client*)arg;

    char receive_buffer[CLIENT_BUFFER_SIZE];
    msock_message result_msg = {
        .buffer = receive_buffer,
        .size = sizeof(receive_buffer)
    };

    while(msock_client_is_connected(client)) {

        ssize_t bytes = msock_client_receive(client, &result_msg);
        if(bytes > 0) {
            lock_chat_state();
            add_message(receive_buffer);
            unlock_chat_state();
        } else if (bytes < 0) {
            lock_chat_state();
            add_message("[Server]: Disconnected!\n");
            unlock_chat_state();
        }
    }

    msock_client_close(client);

    return 0;
}

bool state_chat_init(client_context * c_ctx, char* username)
{
    state_context.username = username;

#ifdef _WIN32
    InitializeCriticalSection(&state_context.lock);
    _beginthreadex(NULL, 0, client_receive_thread, &c_ctx->socket, 0, NULL);
#else
    pthread_mutex_init(&state_context.lock, NULL);
    pthread_t receive_thread;
    pthread_create(&receive_thread, NULL, client_receive_thread, (void*)&c_ctx->socket);
#endif

    return true;
}

void state_chat_tick(client_context *c_ctx)
{
    if (state_context.send_requested) {
        if(handle_client_send(&c_ctx->socket, state_context.input_buffer)) {
            char msg[512];
            snprintf(msg, sizeof(msg), "%s: %s", state_context.username, state_context.input_buffer);
            add_message(msg);
        }
        memset(state_context.input_buffer, 0, sizeof(state_context.input_buffer));
        state_context.send_requested = false;
    }
}

void state_chat_render(mconsole_context *mc_ctx)
{
    mconsole_begin(mc_ctx);

    mconsole_rect screen = mconsole_screen_rect(mc_ctx);
    mconsole_rect input_rect = mconsole_cut_bottom(&screen, 3);
    
    mconsole_draw_border(mc_ctx, input_rect);
    mconsole_rect input_text_area = mconsole_shrink(&input_rect, 1);
    
    if (mconsole_input_field(mc_ctx, input_text_area, state_context.input_buffer, sizeof(state_context.input_buffer))) {
        state_context.send_requested = true; 
    }

    mconsole_draw_border(mc_ctx, screen);
    mconsole_rect content = mconsole_shrink(&screen, 1);
    mconsole_text_box(mc_ctx, content, state_context.message_buffer, true, MCONSOLE_ALIGN_LEFT);

    mconsole_end(mc_ctx);
}

void add_message(char *msg) {
    size_t len = strlen(msg);
    if (state_context.current_message_index + len + 2 >= sizeof(state_context.message_buffer)) {
        state_context.current_message_index = 0;
        state_context.message_buffer[0] = '\0';
    }

    memcpy(&state_context.message_buffer[state_context.current_message_index], msg, len);
    
    state_context.current_message_index += len;
    state_context.message_buffer[state_context.current_message_index++] = '\n';
    state_context.message_buffer[state_context.current_message_index] = '\0';
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

void lock_chat_state() {
#ifdef _WIN32
    EnterCriticalSection(&state_context.lock);
#else
    pthread_mutex_lock(&state_context.lock);
#endif
}

void unlock_chat_state() {
#ifdef _WIN32
    LeaveCriticalSection(&state_context.lock);
#else
    pthread_mutex_unlock(&state_context.lock);
#endif
}