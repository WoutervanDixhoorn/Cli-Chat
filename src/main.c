#include <stdio.h>
#include <string.h>
#include <assert.h>

#define MSOCK_IMPLEMENTATION
#include "msock.h"

#include "client.h"
#include "server.h"

typedef enum {
    MODE_CLIENT = 0,
    MODE_SERVER,
} ChatMode;

static ChatMode mode = 0;

int main(int argc, char **argv) 
{    
    if(argc > 1) {
        char* arg = argv[1];
        printf("Arg: %s\n", arg);
        if(strcmp(arg, "-s") == 0) mode = MODE_SERVER;
    }

    msock_init();

    switch(mode) {
        case MODE_CLIENT: 
            chat_client_run();
            break;
        case MODE_SERVER: 
            chat_server_run();
            break;
        default:
            assert("UNREACHABLE");
    }

    msock_deinit();

    return 0;
}