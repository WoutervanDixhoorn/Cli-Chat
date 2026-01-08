#ifndef MCONSOLE_H
#define MCONSOLE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include <Windows.h>

#define DBG_MAX_LOGS 5
#define DBG_MAX_LEN  128

static char debug_history[DBG_MAX_LOGS][DBG_MAX_LEN];
static int debug_head = 0; // Points to the next write slot

typedef struct {
    char ch;
    uint16_t attr;
} mconsole_cell;

typedef struct {
    int width;
    int height;
    
    size_t buffer_size;

    mconsole_cell* buffer;

    void* internal_buffer;
    void *handle;
    void* input_handle;
    char last_key;

    bool running;
} mconsole_context;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} mconsole_rect;

typedef enum {
    MCONSOLE_ALIGN_LEFT,
    MCONSOLE_ALIGN_RIGHT,
    MCONSOLE_ALIGN_TOP,
    MCONSOLE_ALIGN_BOTTOM,
} mconsole_align;

void mconsole_debug_log(const char *fmt, ...);
void mconsole_draw_debug_overlay(mconsole_context *ctx) ;

bool mconsole_init(mconsole_context *context);
bool mconsole_deinit(mconsole_context *context);

bool mconsole_running(mconsole_context *context);
void mconsole_begin(mconsole_context *context);
void mconsole_end(mconsole_context *context);

mconsole_rect mconsole_screen_rect(mconsole_context *context);
mconsole_rect mconsole_cut_left(mconsole_rect *rect, int amount);
mconsole_rect mconsole_cut_right(mconsole_rect *rect, int amount);
mconsole_rect mconsole_cut_top(mconsole_rect *rect, int amount);
mconsole_rect mconsole_cut_bottom(mconsole_rect *rect, int amount);
mconsole_rect mconsole_shrink(mconsole_rect *rect, int amount);
mconsole_rect mconsole_rect_centered(mconsole_rect parent, int w, int h);

void mconsole_put_cell(mconsole_context *context, int x, int  y, char ch);
void mconsole_draw_border(mconsole_context *context, mconsole_rect rect);
void mconsole_text_box(mconsole_context *context, mconsole_rect rect, char* text, bool text_wrap, mconsole_align align);
bool mconsole_input_field(mconsole_context *context, mconsole_rect rect, char* input_buffer, int buffer_len);

void mconsole_ui_spacer(mconsole_rect *rect, int amount);
bool mconsole_ui_labeld_input(mconsole_context *context, mconsole_rect *rect, char* label, char* input_buffer, int buffer_len, bool active);

#ifdef MCONSOLE_IMPLEMENTATION

void mconsole_debug_log(const char *fmt, ...) 
{
    va_list args;
    va_start(args, fmt);

    vsnprintf(debug_history[debug_head], DBG_MAX_LEN, fmt, args);

    va_end(args);

    debug_head = (debug_head + 1) % DBG_MAX_LOGS;
}

void mconsole_draw_debug_overlay(mconsole_context *ctx) 
{
    mconsole_rect screen = mconsole_screen_rect(ctx);
    
    mconsole_rect log_window = { 
        .x = 0, 
        .y = screen.h - (DBG_MAX_LOGS + 2), 
        .w = 60, 
        .h = DBG_MAX_LOGS + 2 
    };

    mconsole_draw_border(ctx, log_window);
    mconsole_rect content = mconsole_shrink(&log_window, 1);

    for (int i = 0; i < DBG_MAX_LOGS; i++) {
        
        int idx = (debug_head + i) % DBG_MAX_LOGS;

        if (debug_history[idx][0] != '\0') {
            mconsole_rect line = mconsole_cut_top(&content, 1);
            mconsole_text_box(ctx, line, debug_history[idx], false, MCONSOLE_ALIGN_LEFT);
        }
    }
}

bool mconsole_init(mconsole_context *context) {
    HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
    if(h_console == INVALID_HANDLE_VALUE) {
        printf("Failed getting handle to the console\n");
        return false;
    }

    context->handle = (void*)h_console;

    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(h_console, &info);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if(!GetConsoleScreenBufferInfo(h_console, &csbi)) {
        printf("Failed getting screen buffer info\n");
        return false;
    }
    
    context->width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    context->height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    context->buffer_size = context->width * context->height;
    context->buffer = malloc(sizeof(mconsole_cell)*context->buffer_size);
    context->internal_buffer = malloc(sizeof(CHAR_INFO)*context->buffer_size);

    int buffer_size = context->width * context->height;
    COORD draw_coord = {0,0};
    unsigned long chars_written;
    FillConsoleOutputCharacter(
        context->handle,
        (TCHAR) ' ',
        buffer_size,
        draw_coord,
        &chars_written
    );

    FillConsoleOutputAttribute(
        context->handle,
        csbi.wAttributes,
        buffer_size,
        draw_coord,
        &chars_written
    );

    HANDLE h_input = GetStdHandle(STD_INPUT_HANDLE);
    context->input_handle = (void*)h_input;

    DWORD mode;
    GetConsoleMode(h_input, &mode);
    SetConsoleMode(h_input, mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT));

    context->running = true;

    return true;
}

bool mconsole_deinit(mconsole_context *context) {
    HANDLE h_console = (HANDLE)context->handle;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if(!GetConsoleScreenBufferInfo(h_console, &csbi)) {
        printf("Failed getting screen buffer info\n");
        return false;
    }

    int buffer_size = context->width * context->height;
    COORD draw_coord = {0,0};
    unsigned long chars_written;
    FillConsoleOutputCharacter(
        context->handle,
        (TCHAR) ' ',
        buffer_size,
        draw_coord,
        &chars_written
    );

    FillConsoleOutputAttribute(
        context->handle,
        csbi.wAttributes,
        buffer_size,
        draw_coord,
        &chars_written
    );
    
    free(context->buffer);
    free(context->internal_buffer);

    //TODO: Maybe safe first console state and restore.

    return true;
}

bool mconsole_running(mconsole_context *context) {
    if(context->last_key == 27) {
        context->running = false;
    }

    return context->running;
}

void mconsole_begin(mconsole_context *context) {
    context->last_key = 0;

    HANDLE h_input = (HANDLE)context->input_handle;
    DWORD pending;
    GetNumberOfConsoleInputEvents(h_input, &pending);

    if(pending > 0) {
        INPUT_RECORD ir[1];
        DWORD read;
        ReadConsoleInput(h_input, ir, 1, &read);
    
        if(ir[0].EventType == KEY_EVENT) {
            if(ir[0].Event.KeyEvent.bKeyDown) {
                char c = ir[0].Event.KeyEvent.uChar.AsciiChar;
                context->last_key = c ;
            }
        }
    }

    //clear buffer maybe check for resize events?!?
    for(size_t i = 0; i < context->buffer_size; i++) {
        context->buffer[i].ch = ' ';
        context->buffer[i].attr = 0x000F; 
    }
}

void mconsole_end(mconsole_context *context) {
    //Draw to the console
    HANDLE h_console = (HANDLE*)context->handle;
    CHAR_INFO *win_buffer = (CHAR_INFO*)context->internal_buffer;

    //Transform mconsole_cell buffer to CHAR_INFO buffer;
    for(size_t i = 0; i < context->buffer_size; i++) { 
        win_buffer[i].Char.AsciiChar = context->buffer[i].ch;
        win_buffer[i].Attributes     = context->buffer[i].attr;
    }

    COORD buffer_size_coord = {(short)context->width, (short)context->height};  
    COORD draw_coord = {0,0};
    SMALL_RECT write_rect = {
        .Left = 0,
        .Top = 0,
        .Right = (short)context->width - 1,
        .Bottom = (short)context->height - 1
    };

    WriteConsoleOutput(
        h_console,
        win_buffer,
        buffer_size_coord,
        draw_coord,
        &write_rect
    );
}

mconsole_rect mconsole_screen_rect(mconsole_context *context) {
    return (mconsole_rect) {
        .x = 0,
        .y = 0,
        .w = context->width,
        .h = context->height
    };
}

mconsole_rect mconsole_cut_left(mconsole_rect *rect, int amount) {
    mconsole_rect slice = *rect;
    slice.w = amount;
    
    rect->x += amount;
    rect->w -= amount;
    
    return slice;
}

mconsole_rect mconsole_cut_right(mconsole_rect *rect, int amount) {
    rect->w -= (amount - 1);

    return (mconsole_rect) {
        .x = rect->x + rect->w - 1,
        .y = rect->y,
        .w = amount,
        .h = rect->h
    };
}

mconsole_rect mconsole_cut_top(mconsole_rect *rect, int amount) {
    mconsole_rect slice = *rect;
    slice.h = amount;
    
    rect->y += amount;
    rect->h -= amount;
    
    return slice;
}

mconsole_rect mconsole_cut_bottom(mconsole_rect *rect, int amount) {
    rect->h -= (amount - 1);

    return (mconsole_rect) {
        .x = rect->x,
        .y = rect->y + rect->h - 1,
        .w = rect->w,
        .h = amount
    };
}

mconsole_rect mconsole_shrink(mconsole_rect *rect, int amount) {
    return (mconsole_rect) {
        .x = rect->x + amount,
        .y = rect->y + amount,
        .w = rect->w - (amount * 2),
        .h = rect->h - (amount * 2)
    };
}

mconsole_rect mconsole_rect_centered(mconsole_rect parent, int w, int h) {
    mconsole_rect r;
    r.x = parent.x + (parent.w - w) / 2;
    r.y = parent.y + (parent.h - h) / 2;
    r.w = w;
    r.h = h;
    return r;
}

void mconsole_put_cell(mconsole_context *context, int x, int  y, char ch) {
    if (x < 0 || x >= context->width || y < 0 || y >= context->height) {
        return;
    }

    int index = y * context->width + x;
    context->buffer[index].ch = ch;
    context->buffer[index].attr = 0x000F; //TODO: Maybe allow for colors
}

void mconsole_draw_border(mconsole_context *context, mconsole_rect rect) {
    int right = rect.x + rect.w - 1;
    int bottom = rect.y + rect.h - 1;

    for (int x = rect.x; x <= right; x++) {
        mconsole_put_cell(context, x, rect.y, '-'); 
        mconsole_put_cell(context, x, bottom, '-'); 
    }

    for (int y = rect.y; y <= bottom; y++) {
        mconsole_put_cell(context, rect.x, y, '|'); 
        mconsole_put_cell(context, right,  y, '|'); 
    }

    mconsole_put_cell(context, rect.x, rect.y, '+');
    mconsole_put_cell(context, right, rect.y, '+');
    mconsole_put_cell(context, rect.x, bottom, '+');
    mconsole_put_cell(context, right, bottom, '+');
}

void mconsole_text_box(mconsole_context *context, mconsole_rect rect, char* text, bool text_wrap, mconsole_align align) {
    (void) align;

    if(!text) return;
    
    int rel_x = 0;
    int rel_y = 0;
    for(int i = 0; text[i] != '\0'; i++) {
        char c = text[i];
    
        if(c == '\n') {
            rel_x = 0;
            rel_y++;
            continue;
        }

        if(rel_x >= rect.w) {
            if(text_wrap) {
                rel_x = 0;
                rel_y++;
            } else {
                continue;
            }
        }

        if(rel_y >= rect.h) break;

        mconsole_put_cell(context, rect.x + rel_x, rect.y + rel_y, c);
        rel_x++;
    }

    return;
}

bool mconsole_input_field(mconsole_context *context, mconsole_rect rect, char* input_buffer, int buffer_len) {

    int len = (int)strlen(input_buffer);
    bool enter_pressed = false;

    char key = context->last_key;

    if(key != 0) {
        char debug_buf[32];
        sprintf(debug_buf, "Key Code: %d", (int)key);
        SetConsoleTitleA(debug_buf);

        if(key == 8) {
            printf("Backspace!\n");
            if(len > 0) {
                input_buffer[len - 1] = '\0';
            }
        }

        else if(key == 13) {
            enter_pressed = true;
        }

        else if(key >= 32 && key <= 126 && len < buffer_len - 1) {
            input_buffer[len] = key;
            input_buffer[len + 1] = '\0';
        }
    }

    mconsole_text_box(context, rect, input_buffer, false, MCONSOLE_ALIGN_LEFT);

    if(len < rect.w) {
        mconsole_put_cell(context, rect.x + len, rect.y, '_');
    }

    return enter_pressed;
}

void mconsole_ui_spacer(mconsole_rect *rect, int amount) {
    if(amount < 1) return;

    mconsole_cut_top(rect, amount);
}

bool mconsole_ui_labeld_input(mconsole_context *context, mconsole_rect *rect, char* label, char* input_buffer, int buffer_len, bool active) {
    mconsole_rect row_rect = mconsole_cut_top(rect, 1);

    int width = strlen(label) + 1;
    mconsole_rect label_rect = mconsole_cut_left(&row_rect, width); // Have this adjustable
    mconsole_text_box(context, label_rect, label, false, MCONSOLE_ALIGN_LEFT);
    
    if (active) return mconsole_input_field(context, row_rect, input_buffer, buffer_len);

    mconsole_text_box(context, row_rect, input_buffer, true, MCONSOLE_ALIGN_LEFT);
    return false;
}

#endif //MCONSOLE_IMPLEMENTATION
#endif //MCONSOLE_H