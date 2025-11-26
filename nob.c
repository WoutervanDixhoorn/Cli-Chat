#define NOB_IMPLEMENTATION
#include "nob.h"

#define INPUT_FOLDER "src/"
#define OUTPUT_FOLDER "build/"

int compile_chat_program(char* output_file, bool debug) {
    Nob_Cmd cmd = {0};
    nob_cc(&cmd);
    if(debug)
        nob_cmd_append(&cmd, "-g", "-O0");
    nob_cc_flags(&cmd);
    nob_cmd_append(&cmd, "-luser32", "-lWs2_32");
    nob_cmd_append(&cmd, "-o", output_file);
    nob_cc_inputs(&cmd, INPUT_FOLDER"main.c");
    nob_cc_inputs(&cmd, INPUT_FOLDER"client.c");
    nob_cc_inputs(&cmd, INPUT_FOLDER"server.c");

    if(nob_cmd_run_sync(cmd)) return 0;

    return 1;
}

int main(int argc, char** argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool debug = false;
    if(argc > 1 && strcmp(argv[1], "-d") == 0) {
        nob_log(NOB_INFO, "Building with debug symbols");
        debug = true;
    }

    if(!nob_mkdir_if_not_exists(OUTPUT_FOLDER)) {
        printf("Failed creating output dir!\n");
    }

    if(compile_chat_program(OUTPUT_FOLDER"clichat.exe", debug) != 0) printf("Failed building CliChat\n");

    return 0;
}