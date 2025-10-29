#include "../leaf.h"

#include <string.h>
#include <stdio.h>

#include "gui.h"

#define GUICOLOR_TEXT 0.8, 0.7, 0.6
#define GUICOLOR_INPUT_PLACEHOLDER 0.5, 0.5, 0.5
#define GUICOLOR_BUTTON_BG_A (struct color_t){ 60, 60, 65 }
#define GUICOLOR_BUTTON_BG_B (struct color_t){ 20, 20, 35 }
#define GUICOLOR_INPUT_BG_A (struct color_t){ 40, 40, 40 }
#define GUICOLOR_INPUT_BG_B (struct color_t){ 20, 20, 20 }
#define GUICOLOR_INPUT_ACTIVE_BG_A (struct color_t){ 50, 60, 50 }
#define GUICOLOR_INPUT_ACTIVE_BG_B (struct color_t){ 30, 30, 30 }
#define GUICOLOR_BUTTON_INDICATOR_A (struct color_t){ 60, 180, 60 }
#define GUICOLOR_BUTTON_INDICATOR_B (struct color_t){ 30, 80, 30 }
#define TEXT_PADDING_X 4.0
#define TEXT_PADDING_Y 5.0
#define CHECKBOX_TEXT_PADDING 5.0
#define CHECKBOX_TEXT_Y_OFFSET 3.0
#define GUI_AUTOPOS_PADDING_Y 10.0
#define GUI_AUTOPOS_PADDING_X 8.0


static struct GUI_T {
    double mouse_scroll;
    bool mouse_down;

    int active_input_id;
    size_t active_input_memsize;
    char*  active_input_buffer;


    // If LEAFGUI_AUTOPOS is used. These are needed.
    int prev_guielem_pos_x;
    int prev_guielem_pos_y;
    int prev_guielem_width;
    int prev_guielem_height;
    int autopos_home_x;
    int autopos_home_y;

    int flags;
}
GUI = (struct GUI_T) {
    .mouse_scroll = 0,
    .mouse_down = false,
    .active_input_id = -1,
    .active_input_memsize = 0,
    .active_input_buffer = NULL,
    .autopos_home_x = 10,
    .autopos_home_y = 10
};


static struct leaf_ctx_t* g_leaf_ctx = NULL;


static void leafgui_autopos_begin(int* pos_x, int* pos_y) {

    *pos_x = GUI.prev_guielem_pos_x;
    *pos_y = GUI.prev_guielem_pos_y;

    if((GUI.flags & LEAFGUI_SAME_LINE)) {
        *pos_x += GUI.prev_guielem_width;
       
        if(*pos_x > GUI_AUTOPOS_PADDING_X*2) {
            *pos_x += GUI_AUTOPOS_PADDING_X;
        }
    }
    else {
        *pos_y += GUI.prev_guielem_height + GUI_AUTOPOS_PADDING_Y;
    }

    GUI.prev_guielem_pos_x = *pos_x;
    GUI.prev_guielem_pos_y = *pos_y;
}

static void leafgui_autopos_end(int pos_x, int pos_y, int elem_width, int elem_height) {
    GUI.prev_guielem_width = elem_width;
    GUI.prev_guielem_height = elem_height;
}

void leafgui_set_autopos_home_pos(int pos_x, int pos_y) {
    GUI.autopos_home_x = pos_x;
    GUI.autopos_home_y = pos_y;
}

void leafgui_init_context(struct leaf_ctx_t* leaf_ctx) {
    g_leaf_ctx = leaf_ctx;
    GUI.prev_guielem_pos_x = GUI.autopos_home_x;
    GUI.prev_guielem_pos_y = GUI.autopos_home_y;
    GUI.prev_guielem_width = 0;
    GUI.prev_guielem_height = 0;
    GUI.flags = 0;
}

void leafgui_enable(int flags) {
    GUI.flags |= flags;
}
void leafgui_disable(int flags) {
    GUI.flags &= ~flags;
    if((flags & LEAFGUI_SAME_LINE)) {
        GUI.prev_guielem_pos_x = GUI.autopos_home_x;
        GUI.prev_guielem_width = 0;
    }
}

void leafgui_event_mouse_down() {
    GUI.mouse_down = true;
    GUI.active_input_id = -1;
    GUI.active_input_memsize = 0;
    GUI.active_input_buffer = 0;
}

void leafgui_event_mouse_scroll(double yoffset) {
    GUI.mouse_scroll = yoffset;
}

void leafgui_event_key_input(int key) {
    if((GUI.active_input_id < 0) ||
        !GUI.active_input_buffer ||
        !GUI.active_input_memsize) {
        return;
    }

    if(key == GLFW_KEY_BACKSPACE) {
        const size_t buffer_len = strlen(GUI.active_input_buffer);
        if(buffer_len > 0) {
            GUI.active_input_buffer[buffer_len-1] = 0;
        }
    }
}

void leafgui_event_char_input(char ch) {
    if((ch < 0x20) || (ch > 0x7E)) {
        return; // Not ASCII character.
    }
    if((GUI.active_input_id < 0) ||
        !GUI.active_input_buffer ||
        !GUI.active_input_memsize) {
        return;
    }

    const size_t buffer_len = strlen(GUI.active_input_buffer);
    if(buffer_len+1 >= GUI.active_input_memsize) {
        return;
    }

    GUI.active_input_buffer[buffer_len] = ch;
    
}

void leafgui_end_update() {
    GUI.mouse_down = false;
    GUI.mouse_scroll = 0;
    GUI.prev_guielem_pos_x = 10;
    GUI.prev_guielem_pos_y = 10;
    GUI.prev_guielem_width = 0;
    GUI.prev_guielem_height = 0;
    GUI.flags = 0;
}


static bool mouse_on_rect(int x, int y, int w, int h) {
    double mouse_x = 0;
    double mouse_y = 0;
    glfwGetCursorPos(g_leaf_ctx->glfw_win, &mouse_x, &mouse_y);

    return ((int)mouse_x > x && (int)mouse_x < x+w) && ((int)mouse_y > y && (int)mouse_y < y+h);
}


bool leafgui_button
(
    struct font_t* font,
    char* label,
    int pos_x,
    int pos_y
){
    const bool auto_pos = (pos_x < 0 && pos_y < 0);

    if(auto_pos) {
        leafgui_autopos_begin(&pos_x, &pos_y);
    }

    const size_t label_len = strlen(label);

    int text_width;
    int text_height;
    leaf_measure_text(font, &text_width, &text_height, label, label_len);

    const int rect_w = text_width + TEXT_PADDING_X * 2;
    const int rect_h = text_height + TEXT_PADDING_Y;
    
    const bool mouse_on = mouse_on_rect(pos_x, pos_y, rect_w, rect_h);

    if(auto_pos) {
        leafgui_autopos_end(pos_x, pos_y, rect_w, rect_h);
    }

    // Indicator
    if(mouse_on) {
        leaf_draw_rect_fade(pos_x-2, pos_y-2, rect_w+4, rect_h+4,
                GUICOLOR_BUTTON_INDICATOR_A,
                GUICOLOR_BUTTON_INDICATOR_B,
                LEAF_RECT_FADE_HORIZONTAL);       
    }

    // Background.
    leaf_draw_rect_fade(pos_x, pos_y, rect_w, rect_h,
            GUICOLOR_BUTTON_BG_A,
            GUICOLOR_BUTTON_BG_B,
            LEAF_RECT_FADE_HORIZONTAL);

    leaf_set_font_color(font, GUICOLOR_TEXT);
    leaf_draw_text(font,
            pos_x + TEXT_PADDING_X,
            pos_y + TEXT_PADDING_Y,
            label, label_len);

    return (mouse_on && GUI.mouse_down);
}

bool leafgui_checkbox
(
    struct font_t* font,
    char* label,
    int pos_x,
    int pos_y,
    bool* value_ptr
){
    const size_t label_len = strlen(label);
    const bool auto_pos = (pos_x < 0 && pos_y < 0);
    if(auto_pos) {
        leafgui_autopos_begin(&pos_x, &pos_y);
    }
        
    int text_width;
    int text_height;
    leaf_measure_text(font, &text_width, &text_height, label, label_len);

    const int box_w = font->char_height / 2;
    const int box_h = font->char_height / 2;
    const int rect_w = box_w + text_width + TEXT_PADDING_X + CHECKBOX_TEXT_PADDING;
    const int rect_h = box_h;
    const bool mouse_on = mouse_on_rect(pos_x, pos_y, rect_w, rect_h);

    if(auto_pos) {
        leafgui_autopos_end(pos_x, pos_y, rect_w, rect_h);
    }

    // Background.
    leaf_draw_rect_fade(pos_x, pos_y, box_w, box_h,
            GUICOLOR_BUTTON_BG_A,
            GUICOLOR_BUTTON_BG_B,
            LEAF_RECT_FADE_HORIZONTAL);
 
    // Indicator.
    if(*value_ptr) {
        leaf_draw_rect_fade(pos_x+2, pos_y+2, box_w-4, box_h-4,
                GUICOLOR_BUTTON_INDICATOR_A,
                GUICOLOR_BUTTON_INDICATOR_B,
                LEAF_RECT_FADE_HORIZONTAL);    
    }   

    leaf_set_font_color(font, GUICOLOR_TEXT);
    leaf_draw_text(font,
            pos_x + TEXT_PADDING_X + box_w + CHECKBOX_TEXT_PADDING,
            pos_y + TEXT_PADDING_Y - CHECKBOX_TEXT_Y_OFFSET,
            label, label_len);

    const bool active = (mouse_on && GUI.mouse_down);
    if(active) {
        *value_ptr = !(*value_ptr);
    }
    return active;
}

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
){
    const bool auto_pos = (pos_x < 0 && pos_y < 0); 

    if(auto_pos) {
        leafgui_autopos_begin(&pos_x, &pos_y);    
    }

    const size_t place_holder_len = strlen(place_holder);
    const size_t buffer_len = strlen(buffer);

    const int rect_w = width + TEXT_PADDING_X;
    const int rect_h = font->char_height/2 + TEXT_PADDING_Y;
    
    const bool mouse_on = mouse_on_rect(pos_x, pos_y, rect_w, rect_h);

    if(auto_pos) {
        leafgui_autopos_end(pos_x, pos_y, rect_w, rect_h);
    }

    // Indicator.
    if(mouse_on) {
        leaf_draw_rect_fade(pos_x-2, pos_y-2, rect_w+4, rect_h+4,
                GUICOLOR_BUTTON_INDICATOR_A,
                GUICOLOR_BUTTON_INDICATOR_B,
                LEAF_RECT_FADE_HORIZONTAL);    
    }

    if(mouse_on && GUI.mouse_down) {
        GUI.active_input_id = id;
        GUI.active_input_memsize = buffer_memsize;
        GUI.active_input_buffer = buffer;
    }


    bool active = (id == GUI.active_input_id);

    // Background.
    leaf_draw_rect_fade(pos_x, pos_y, rect_w, rect_h,
            active ? GUICOLOR_INPUT_ACTIVE_BG_A : GUICOLOR_INPUT_BG_A,
            active ? GUICOLOR_INPUT_ACTIVE_BG_B : GUICOLOR_INPUT_BG_B,
            LEAF_RECT_FADE_HORIZONTAL);

    if(buffer_len == 0) {
        leaf_set_font_color(font, GUICOLOR_INPUT_PLACEHOLDER);
        leaf_draw_text(font,
                pos_x + TEXT_PADDING_X,
                pos_y + TEXT_PADDING_Y,
                place_holder, -1);
    }
    else {
        leaf_set_font_color(font, GUICOLOR_TEXT);
        leaf_draw_text(font,
                pos_x + TEXT_PADDING_X,
                pos_y + TEXT_PADDING_Y,
                buffer, buffer_len);

    }



    return false;
}


