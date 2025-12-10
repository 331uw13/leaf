#ifndef LEAF_GUI_H
#define LEAF_GUI_H

#include <stdbool.h>
#include <stdint.h>

struct font_t; 
struct leaf_ctx_t;


void leafgui_init_context(struct leaf_ctx_t* leaf_ctx);


void leafgui_event_mouse_down();
void leafgui_event_mouse_scroll(double yoffset);
void leafgui_event_key_input(int key);
void leafgui_event_char_input(char ch);

// Call this function after done updating the gui. It will clear event flags.
void leafgui_end_update(); 


// Flags.
#define LEAFGUI_SAME_LINE (1 << 0)

void leafgui_enable(int flags);
void leafgui_disable(int flags);

// Where auto pos will start.
void leafgui_set_autopos_home_pos(int pos_x, int pos_y);

// When pos_x and pos_y are negative for gui elements.
// The gui element will get position automatically.
#define LEAFGUI_AUTOPOS -1, -1

bool leafgui_button
(
    struct font_t* font,
    char* label,
    int pos_x,
    int pos_y
);

bool leafgui_checkbox
(
    struct font_t* font,
    char* label,
    int pos_x,
    int pos_y,
    bool* value_ptr
);

bool leafgui_input
(
    uint32_t id,
    struct font_t* font,
    char* place_holder,
    int pos_x,
    int pos_y,
    int width,
    char* buffer,
    size_t buffer_memsize
);



#endif
