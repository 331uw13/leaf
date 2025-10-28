

#include "leaf/leaf.h"




int main(int argc, char** argv) {

    struct leaf_ctx_t* leaf_ctx = leaf_open("test window", 800, 600);
    if(!leaf_ctx) {
        return 1;
    }

    
    const size_t renderer_max_vertices = 1024*8;
    leaf_init_renderer(leaf_ctx, renderer_max_vertices);

    //const char* font_file = "Topaz-8.ttf";
    const char* font_file = "DejaVuSansCondensed.ttf";


    struct font_t font;
    if(!leaf_load_font(&font, font_file)) {
        return 1;
    }


    // Font settings can be changed at any time.
    //
    // leaf_set_font_tab_width(&font, 16 * 4);
    // leaf_set_font_space_width(&font, 16);
    // leaf_set_font_spacing(&font, 8);
    
    while(!glfwWindowShouldClose(leaf_ctx->glfw_win)) {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        
        leaf_set_font_scale(&font, 1.6);
        leaf_set_font_color(&font, 1.0, 0.8, 0.6);
        leaf_draw_text(&font, 10, 30, "Rendering text using Freetype2, GLFW and OpenGL", -1);

        leaf_set_font_scale(&font, 3.0);
        leaf_set_font_color(&font, 1.0, 1.0, 1.0);
        leaf_draw_text(&font, 350, 60, "Another big text", -1);

        leaf_set_font_scale(&font, 1.3);
        leaf_draw_text(&font, 350, 120, "... and small text", -1);


        leaf_set_font_scale(&font, 1.8);
        leaf_set_font_color(&font, 0.3, 1.0, 0.3);
        leaf_draw_text(&font, 40, leaf_ctx->win_height-60, "supports only ASCII characters.", -1);


        leaf_draw_rect_fade(100, 200, 200, 200,
                (struct color_t){ 255, 30, 30 },
                (struct color_t){ 30, 30, 255 },
                LEAF_RECT_FADE_HORIZONTAL);


        leaf_draw_circle(300, 200, 50.0f, 8, (struct color_t){ 50, 255, 200 });
        leaf_draw_circle(380, 330, 43.5f, 32, (struct color_t){ 255, 170, 20 });

        leaf_render_vertices(leaf_ctx);

        leaf_clear_vertices(leaf_ctx);
        glfwSwapBuffers(leaf_ctx->glfw_win);
        glfwWaitEvents();
    }


    leaf_unload_font(&font);
    leaf_free_renderer(leaf_ctx);
    leaf_quit(leaf_ctx);
    return 0;
}



