#include "mcigraph.hpp"

int mandelbrot(double x0, double y0) {
    double x = 0.0, y = 0.0;
    int i = 0;
    while (x * x + y * y <= 4 && i++ < 32){
        double xtemp = x * x - y * y + x0;
        y = 2 * x * y + y0;
        x = xtemp;
    }
    return i;
}

unsigned int get_color(int iter) {
    unsigned int colors[] = {3939855, 1640218, 590127, 263241, 1892, 797834, 1594033, 3767761, 8828389, 13888760, 15854015, 16304479, 16755200, 13402112, 10049280, 6960131};
    return colors[iter % 16];
}

void color_to_rgb(unsigned int color, unsigned int* r, unsigned int* g, unsigned int* b) {
    *r = (color & 0x00ff0000) >> 16;
    *g = (color & 0x0000ff00) >> 8;
    *b = (color & 0x000000ff) >> 0;
}

int main() {
    while (running()) {
        if(is_pressed(KEY_SPACE)) toggle_fullscreen();
        
        begin_drawing();

        // Calculate Mandelbrot
        for (int y = 0; y < get_screen_height(); y++) {
            for (int x = 0; x < get_screen_width(); x++) {
                // Scale pixel coordinates to mandel brot range
                double x0 = (x / (double)get_screen_width()) * 4.5 - 2.75;
                double y0 = (y / (double)get_screen_height())* 2.8 - 1.4;
                // Calculate number of iterations until divergence
                int iter = mandelbrot(x0, y0);
                // Determine color based on number of iterations
                unsigned int color = get_color(iter);
                unsigned int r, g, b;
                color_to_rgb(color, &r, &g, &b);
                draw_point(x, y, r, g, b);
            }
        }

        // Print frames per second in upper left corner
        double fps = 1.0 / get_delta_time();
        char buffer[9+1];
        sprintf(buffer, "FPS: %04.1f", fps);
        draw_text(buffer, 10, 10, 32, 0xff, 0xff, 0xff);

        end_drawing();
    }
    return 0;
}
