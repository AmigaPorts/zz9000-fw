#include <stdint.h>

typedef struct Vec2 {
	float x;
	float y;
} Vec2;

void set_fb(uint32_t* fb_, uint32_t pitch);
void horizline(uint16_t x1, uint16_t x2, uint16_t y, uint32_t color);
void fill_rect(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint32_t rect_rgb);
void fill_triangle(Vec2 A, Vec2 B, Vec2 C, uint32_t color);
void render_faces();
