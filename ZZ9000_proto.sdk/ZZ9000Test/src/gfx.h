/*
 * MNT ZZ9000 Amiga Graphics and Coprocessor Card Operating System (ZZ9000OS)
 *
 * Copyright (C) 2019, Lukas F. Hartmann <lukas@mntre.com>
 *                     MNT Research GmbH, Berlin
 *                     https://mntre.com
 *
 * More Info: https://mntre.com/zz9000
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 *
 * https://spdx.org/licenses/GPL-3.0-or-later.html
 *
*/

#include <stdint.h>

typedef struct Vec2 {
	float x;
	float y;
} Vec2;

void set_fb(uint32_t* fb_, uint32_t pitch);
void horizline(uint16_t x1, uint16_t x2, uint16_t y, uint32_t color);

void fill_rect8(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint8_t rect_rgb);
void fill_rect16(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint16_t rect_rgb);
void fill_rect32(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint32_t rect_rgb);

void copy_rect8(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint16_t rect_sx, uint16_t rect_sy);
void copy_rect16(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint16_t rect_sx, uint16_t rect_sy);
void copy_rect32(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint16_t rect_sx, uint16_t rect_sy);

void fill_template(uint32_t bpp, uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2,
		uint8_t draw_mode, uint8_t mask, uint32_t fg_color, uint32_t bg_color, uint16_t x_offset, uint16_t y_offset, uint8_t* tmpl_data, uint16_t templ_pitch, uint16_t loop_rows);

