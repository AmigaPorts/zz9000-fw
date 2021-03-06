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
void update_hw_sprite(uint8_t *data, uint32_t *colors, uint16_t w, uint16_t h);
void clip_hw_sprite(int16_t offset_x, int16_t offset_y);
void clear_hw_sprite();
void horizline(uint16_t x1, uint16_t x2, uint16_t y, uint32_t color);

void fill_rect(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint32_t rect_rgb, uint32_t color_format, uint8_t mask);
void fill_rect_solid(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint32_t rect_rgb, uint32_t color_format);
void fill_rect8(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint8_t rect_rgb);
void fill_rect16(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint16_t rect_rgb);
void fill_rect32(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint32_t rect_rgb);

void copy_rect(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint16_t rect_sx, uint16_t rect_sy, uint32_t color_format, uint32_t* sp_src, uint32_t src_pitch, uint8_t mask);
void copy_rect_nomask(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint16_t rect_sx, uint16_t rect_sy, uint32_t color_format, uint32_t* sp_src, uint32_t src_pitch, uint8_t draw_mode);
void copy_rect8(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint16_t rect_sx, uint16_t rect_sy);
void copy_rect16(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint16_t rect_sx, uint16_t rect_sy);
void copy_rect32(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint16_t rect_sx, uint16_t rect_sy);

void fill_template(uint32_t bpp, uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2,
		uint8_t draw_mode, uint8_t mask, uint32_t fg_color, uint32_t bg_color, uint16_t x_offset, uint16_t y_offset, uint8_t* tmpl_data, uint16_t templ_pitch, uint16_t loop_rows);
void template_fill_rect(uint32_t color_format, uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h,
	uint8_t draw_mode, uint8_t mask, uint32_t fg_color, uint32_t bg_color,
	uint16_t x_offset, uint16_t y_offset,
	uint8_t *tmpl_data, uint16_t tmpl_pitch);
void pattern_fill_rect(uint32_t color_format, uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h,
	uint8_t draw_mode, uint8_t mask, uint32_t fg_color, uint32_t bg_color,
	uint16_t x_offset, uint16_t y_offset,
	uint8_t *tmpl_data, uint16_t tmpl_pitch, uint16_t loop_rows);

void draw_line(int16_t rect_x1, int16_t rect_y1, int16_t rect_x2, int16_t rect_y2, uint16_t len, uint16_t pattern, uint16_t pattern_offset, uint32_t fg_color, uint32_t bg_color, uint32_t color_format, uint8_t mask, uint8_t draw_mode);
void draw_line_solid(int16_t rect_x1, int16_t rect_y1, int16_t rect_x2, int16_t rect_y2, uint16_t len, uint32_t fg_color, uint32_t color_format);

void p2c_rect(int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t w, int16_t h, uint16_t sh, uint8_t draw_mode, uint8_t planes, uint8_t mask, uint8_t layer_mask, uint16_t src_line_pitch, uint8_t *bmp_data_src);
void p2d_rect(int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t w, int16_t h, uint16_t sh, uint8_t draw_mode, uint8_t planes, uint8_t mask, uint8_t layer_mask, uint32_t color_mask, uint16_t src_line_pitch, uint8_t *bmp_data_src, uint32_t color_format);
void invert_rect(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint8_t mask, uint32_t color_format);

#define MNTVA_COLOR_8BIT     0
#define MNTVA_COLOR_16BIT565 1
#define MNTVA_COLOR_32BIT    2
#define MNTVA_COLOR_1BIT     3
#define MNTVA_COLOR_15BIT    4

// see http://amigadev.elowar.com/read/ADCD_2.1/Libraries_Manual_guide/node0351.html
#define JAM1	    0	      /* jam 1 color into raster */
#define JAM2	    1	      /* jam 2 colors into raster */
#define COMPLEMENT  2	      /* XOR bits into raster */
#define INVERSVID   4	      /* inverse video for drawing modes */

// Graphics minterm drawing modes
enum gfx_minterm_modes {
	MINTERM_FALSE,
	MINTERM_NOR,
	MINTERM_ONLYDST,
	MINTERM_NOTSRC,
	MINTERM_ONLYSRC,
	MINTERM_INVERT,
	MINTERM_EOR,
	MINTERM_NAND,
	MINTERM_AND,
	MINTERM_NEOR,
	MINTERM_DST,
	MINTERM_NOTONLYSRC,
	MINTERM_SRC,
	MINTERM_NOTONLYDST,
	MINTERM_OR,
	MINTERM_TRUE,
};

/* Macros for keeping gfx.c a bit more tidy */
#define SET_FG_PIXEL8(a) \
	((uint8_t *)dp)[x+a] = u8_fg;
#define SET_FG_PIXEL16(a) \
	((uint16_t *)dp)[x+a] = fg_color;
#define SET_FG_PIXEL32(a) \
	dp[x+a] = fg_color;

#define SET_BG_PIXEL8(a) \
	((uint8_t *)dp)[x+a] = u8_bg;
#define SET_BG_PIXEL16(a) \
	((uint16_t *)dp)[x+a] = bg_color;
#define SET_BG_PIXEL32(a) \
	dp[x+a] = bg_color;

#define SET_FG_PIXEL8_MASK(a) \
	((uint8_t *)dp)[x + a] = u8_fg ^ (((uint8_t *)dp)[x + a] & (mask ^ 0xFF));
#define SET_BG_PIXEL8_MASK(a) \
	((uint8_t *)dp)[x + a] = u8_bg ^ (((uint8_t *)dp)[x + a] & (mask ^ 0xFF));
#define SET_FG_PIXEL16_MASK(a) \
	((uint16_t *)dp)[x + a] = fg_color ^ (((uint16_t *)dp)[x + a] & (color_mask ^ 0xFFFF));
#define SET_FG_PIXEL32_MASK(a) \
	dp[x + a] = fg_color ^ (dp[x + a] & (color_mask ^ 0xFFFFFFFF));

#define SET_FG_PIXEL \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			SET_FG_PIXEL8(0); break; \
		case MNTVA_COLOR_16BIT565: \
			SET_FG_PIXEL16(0); break; \
		case MNTVA_COLOR_32BIT: \
			SET_FG_PIXEL32(0); break; \
	}

#define SET_FG_PIXEL_MASK \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			SET_FG_PIXEL8_MASK(0); break; \
		case MNTVA_COLOR_16BIT565: \
			SET_FG_PIXEL16(0); break; \
		case MNTVA_COLOR_32BIT: \
			SET_FG_PIXEL32(0); break; \
	}

#define SET_BG_PIXEL \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			SET_BG_PIXEL8(0); break; \
		case MNTVA_COLOR_16BIT565: \
			SET_BG_PIXEL16(0); break; \
		case MNTVA_COLOR_32BIT: \
			SET_BG_PIXEL32(0); break; \
	}

#define SET_BG_PIXEL_MASK \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			SET_BG_PIXEL8_MASK(0); break; \
		case MNTVA_COLOR_16BIT565: \
			SET_BG_PIXEL16(0); break; \
		case MNTVA_COLOR_32BIT: \
			SET_BG_PIXEL32(0); break; \
	}

#define SET_FG_PIXELS \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL8(0); \
			if (cur_byte & 0x40) SET_FG_PIXEL8(1); \
			if (cur_byte & 0x20) SET_FG_PIXEL8(2); \
			if (cur_byte & 0x10) SET_FG_PIXEL8(3); \
			if (cur_byte & 0x08) SET_FG_PIXEL8(4); \
			if (cur_byte & 0x04) SET_FG_PIXEL8(5); \
			if (cur_byte & 0x02) SET_FG_PIXEL8(6); \
			if (cur_byte & 0x01) SET_FG_PIXEL8(7); \
			break; \
		case MNTVA_COLOR_16BIT565: \
			if (cur_byte & 0x80) SET_FG_PIXEL16(0); \
			if (cur_byte & 0x40) SET_FG_PIXEL16(1); \
			if (cur_byte & 0x20) SET_FG_PIXEL16(2); \
			if (cur_byte & 0x10) SET_FG_PIXEL16(3); \
			if (cur_byte & 0x08) SET_FG_PIXEL16(4); \
			if (cur_byte & 0x04) SET_FG_PIXEL16(5); \
			if (cur_byte & 0x02) SET_FG_PIXEL16(6); \
			if (cur_byte & 0x01) SET_FG_PIXEL16(7); \
			break; \
		case MNTVA_COLOR_32BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL32(0); \
			if (cur_byte & 0x40) SET_FG_PIXEL32(1); \
			if (cur_byte & 0x20) SET_FG_PIXEL32(2); \
			if (cur_byte & 0x10) SET_FG_PIXEL32(3); \
			if (cur_byte & 0x08) SET_FG_PIXEL32(4); \
			if (cur_byte & 0x04) SET_FG_PIXEL32(5); \
			if (cur_byte & 0x02) SET_FG_PIXEL32(6); \
			if (cur_byte & 0x01) SET_FG_PIXEL32(7); \
			break; \
	}

#define SET_FG_PIXELS_MASK \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL8_MASK(0); \
			if (cur_byte & 0x40) SET_FG_PIXEL8_MASK(1); \
			if (cur_byte & 0x20) SET_FG_PIXEL8_MASK(2); \
			if (cur_byte & 0x10) SET_FG_PIXEL8_MASK(3); \
			if (cur_byte & 0x08) SET_FG_PIXEL8_MASK(4); \
			if (cur_byte & 0x04) SET_FG_PIXEL8_MASK(5); \
			if (cur_byte & 0x02) SET_FG_PIXEL8_MASK(6); \
			if (cur_byte & 0x01) SET_FG_PIXEL8_MASK(7); \
			break; \
		case MNTVA_COLOR_16BIT565: \
			if (cur_byte & 0x80) SET_FG_PIXEL16(0); \
			if (cur_byte & 0x40) SET_FG_PIXEL16(1); \
			if (cur_byte & 0x20) SET_FG_PIXEL16(2); \
			if (cur_byte & 0x10) SET_FG_PIXEL16(3); \
			if (cur_byte & 0x08) SET_FG_PIXEL16(4); \
			if (cur_byte & 0x04) SET_FG_PIXEL16(5); \
			if (cur_byte & 0x02) SET_FG_PIXEL16(6); \
			if (cur_byte & 0x01) SET_FG_PIXEL16(7); \
			break; \
		case MNTVA_COLOR_32BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL32(0); \
			if (cur_byte & 0x40) SET_FG_PIXEL32(1); \
			if (cur_byte & 0x20) SET_FG_PIXEL32(2); \
			if (cur_byte & 0x10) SET_FG_PIXEL32(3); \
			if (cur_byte & 0x08) SET_FG_PIXEL32(4); \
			if (cur_byte & 0x04) SET_FG_PIXEL32(5); \
			if (cur_byte & 0x02) SET_FG_PIXEL32(6); \
			if (cur_byte & 0x01) SET_FG_PIXEL32(7); \
			break; \
	}

#define SET_FG_OR_BG_PIXELS \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL8(0) else SET_BG_PIXEL8(0) \
			if (cur_byte & 0x40) SET_FG_PIXEL8(1) else SET_BG_PIXEL8(1) \
			if (cur_byte & 0x20) SET_FG_PIXEL8(2) else SET_BG_PIXEL8(2) \
			if (cur_byte & 0x10) SET_FG_PIXEL8(3) else SET_BG_PIXEL8(3) \
			if (cur_byte & 0x08) SET_FG_PIXEL8(4) else SET_BG_PIXEL8(4) \
			if (cur_byte & 0x04) SET_FG_PIXEL8(5) else SET_BG_PIXEL8(5) \
			if (cur_byte & 0x02) SET_FG_PIXEL8(6) else SET_BG_PIXEL8(6) \
			if (cur_byte & 0x01) SET_FG_PIXEL8(7) else SET_BG_PIXEL8(7) \
			break; \
		case MNTVA_COLOR_16BIT565: \
			if (cur_byte & 0x80) SET_FG_PIXEL16(0) else SET_BG_PIXEL16(0) \
			if (cur_byte & 0x40) SET_FG_PIXEL16(1) else SET_BG_PIXEL16(1) \
			if (cur_byte & 0x20) SET_FG_PIXEL16(2) else SET_BG_PIXEL16(2) \
			if (cur_byte & 0x10) SET_FG_PIXEL16(3) else SET_BG_PIXEL16(3) \
			if (cur_byte & 0x08) SET_FG_PIXEL16(4) else SET_BG_PIXEL16(4) \
			if (cur_byte & 0x04) SET_FG_PIXEL16(5) else SET_BG_PIXEL16(5) \
			if (cur_byte & 0x02) SET_FG_PIXEL16(6) else SET_BG_PIXEL16(6) \
			if (cur_byte & 0x01) SET_FG_PIXEL16(7) else SET_BG_PIXEL16(7) \
			break; \
		case MNTVA_COLOR_32BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL32(0) else SET_BG_PIXEL32(0) \
			if (cur_byte & 0x40) SET_FG_PIXEL32(1) else SET_BG_PIXEL32(1) \
			if (cur_byte & 0x20) SET_FG_PIXEL32(2) else SET_BG_PIXEL32(2) \
			if (cur_byte & 0x10) SET_FG_PIXEL32(3) else SET_BG_PIXEL32(3) \
			if (cur_byte & 0x08) SET_FG_PIXEL32(4) else SET_BG_PIXEL32(4) \
			if (cur_byte & 0x04) SET_FG_PIXEL32(5) else SET_BG_PIXEL32(5) \
			if (cur_byte & 0x02) SET_FG_PIXEL32(6) else SET_BG_PIXEL32(6) \
			if (cur_byte & 0x01) SET_FG_PIXEL32(7) else SET_BG_PIXEL32(7) \
			break; \
	}

#define SET_FG_OR_BG_PIXELS_MASK \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL8_MASK(0) else SET_BG_PIXEL8_MASK(0) \
			if (cur_byte & 0x40) SET_FG_PIXEL8_MASK(1) else SET_BG_PIXEL8_MASK(1) \
			if (cur_byte & 0x20) SET_FG_PIXEL8_MASK(2) else SET_BG_PIXEL8_MASK(2) \
			if (cur_byte & 0x10) SET_FG_PIXEL8_MASK(3) else SET_BG_PIXEL8_MASK(3) \
			if (cur_byte & 0x08) SET_FG_PIXEL8_MASK(4) else SET_BG_PIXEL8_MASK(4) \
			if (cur_byte & 0x04) SET_FG_PIXEL8_MASK(5) else SET_BG_PIXEL8_MASK(5) \
			if (cur_byte & 0x02) SET_FG_PIXEL8_MASK(6) else SET_BG_PIXEL8_MASK(6) \
			if (cur_byte & 0x01) SET_FG_PIXEL8_MASK(7) else SET_BG_PIXEL8_MASK(7) \
			break; \
		case MNTVA_COLOR_16BIT565: \
			if (cur_byte & 0x80) SET_FG_PIXEL16(0) else SET_BG_PIXEL16(0) \
			if (cur_byte & 0x40) SET_FG_PIXEL16(1) else SET_BG_PIXEL16(1) \
			if (cur_byte & 0x20) SET_FG_PIXEL16(2) else SET_BG_PIXEL16(2) \
			if (cur_byte & 0x10) SET_FG_PIXEL16(3) else SET_BG_PIXEL16(3) \
			if (cur_byte & 0x08) SET_FG_PIXEL16(4) else SET_BG_PIXEL16(4) \
			if (cur_byte & 0x04) SET_FG_PIXEL16(5) else SET_BG_PIXEL16(5) \
			if (cur_byte & 0x02) SET_FG_PIXEL16(6) else SET_BG_PIXEL16(6) \
			if (cur_byte & 0x01) SET_FG_PIXEL16(7) else SET_BG_PIXEL16(7) \
			break; \
		case MNTVA_COLOR_32BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL32(0) else SET_BG_PIXEL32(0) \
			if (cur_byte & 0x40) SET_FG_PIXEL32(1) else SET_BG_PIXEL32(1) \
			if (cur_byte & 0x20) SET_FG_PIXEL32(2) else SET_BG_PIXEL32(2) \
			if (cur_byte & 0x10) SET_FG_PIXEL32(3) else SET_BG_PIXEL32(3) \
			if (cur_byte & 0x08) SET_FG_PIXEL32(4) else SET_BG_PIXEL32(4) \
			if (cur_byte & 0x04) SET_FG_PIXEL32(5) else SET_BG_PIXEL32(5) \
			if (cur_byte & 0x02) SET_FG_PIXEL32(6) else SET_BG_PIXEL32(6) \
			if (cur_byte & 0x01) SET_FG_PIXEL32(7) else SET_BG_PIXEL32(7) \
			break; \
	}

#define INVERT_PIXEL \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			((uint8_t *)dp)[x] ^= mask; break; \
		case MNTVA_COLOR_16BIT565: \
			((uint16_t *)dp)[x] ^= 0xFFFF; break; \
		case MNTVA_COLOR_32BIT: \
			dp[x] ^= 0xFFFFFFFF; break; \
	}

#define INVERT_PIXEL_FG \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			((uint8_t *)dp)[x] = u8_fg ^ 0xFF; break; \
		case MNTVA_COLOR_16BIT565: \
			((uint16_t *)dp)[x] ^= fg_color; break; \
		case MNTVA_COLOR_32BIT: \
			dp[x] ^= fg_color; break; \
	}

#define INVERT_PIXEL_BG \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			((uint8_t *)dp)[x] ^= u8_bg; break; \
		case MNTVA_COLOR_16BIT565: \
			((uint16_t *)dp)[x] ^= bg_color; break; \
		case MNTVA_COLOR_32BIT: \
			dp[x] ^= bg_color; break; \
	}

#define INVERT_PIXELS \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			if (cur_byte & 0x80) ((uint8_t *)dp)[x] ^= mask; \
			if (cur_byte & 0x40) ((uint8_t *)dp)[x+1] ^= mask; \
			if (cur_byte & 0x20) ((uint8_t *)dp)[x+2] ^= mask; \
			if (cur_byte & 0x10) ((uint8_t *)dp)[x+3] ^= mask; \
			if (cur_byte & 0x08) ((uint8_t *)dp)[x+4] ^= mask; \
			if (cur_byte & 0x04) ((uint8_t *)dp)[x+5] ^= mask; \
			if (cur_byte & 0x02) ((uint8_t *)dp)[x+6] ^= mask; \
			if (cur_byte & 0x01) ((uint8_t *)dp)[x+7] ^= mask; \
			break; \
		case MNTVA_COLOR_16BIT565: \
			if (cur_byte & 0x80) ((uint16_t *)dp)[x] ^= 0xFFFF; \
			if (cur_byte & 0x40) ((uint16_t *)dp)[x+1] ^= 0xFFFF; \
			if (cur_byte & 0x20) ((uint16_t *)dp)[x+2] ^= 0xFFFF; \
			if (cur_byte & 0x10) ((uint16_t *)dp)[x+3] ^= 0xFFFF; \
			if (cur_byte & 0x08) ((uint16_t *)dp)[x+4] ^= 0xFFFF; \
			if (cur_byte & 0x04) ((uint16_t *)dp)[x+5] ^= 0xFFFF; \
			if (cur_byte & 0x02) ((uint16_t *)dp)[x+6] ^= 0xFFFF; \
			if (cur_byte & 0x01) ((uint16_t *)dp)[x+7] ^= 0xFFFF; \
			break; \
		case MNTVA_COLOR_32BIT: \
			if (cur_byte & 0x80) dp[x] ^= 0xFFFFFFFF; \
			if (cur_byte & 0x40) dp[x+1] ^= 0xFFFFFFFF; \
			if (cur_byte & 0x20) dp[x+2] ^= 0xFFFFFFFF; \
			if (cur_byte & 0x10) dp[x+3] ^= 0xFFFFFFFF; \
			if (cur_byte & 0x08) dp[x+4] ^= 0xFFFFFFFF; \
			if (cur_byte & 0x04) dp[x+5] ^= 0xFFFFFFFF; \
			if (cur_byte & 0x02) dp[x+6] ^= 0xFFFFFFFF; \
			if (cur_byte & 0x01) dp[x+7] ^= 0xFFFFFFFF; \
			break; \
	}

#define HANDLE_MINTERM_PIXEL_8(s, d) \
	switch(draw_mode) {\
		case MINTERM_NOR: \
			s &= ~(d); \
			SET_FG_PIXEL8_MASK(0); break; \
		case MINTERM_ONLYDST: \
			d = d & ~(s); break; \
		case MINTERM_NOTSRC: \
			SET_FG_PIXEL8_MASK(0); break; \
		case MINTERM_ONLYSRC: \
			s &= (d ^ 0xFF); \
			SET_FG_PIXEL8_MASK(0); break; \
		case MINTERM_INVERT: \
			d ^= 0xFF; break; \
		case MINTERM_EOR: \
			d ^= s; break; \
		case MINTERM_NAND: \
			s = ~(d & ~(s)) & mask; \
			SET_FG_PIXEL8_MASK(0); break; \
		case MINTERM_AND: \
			s &= d; \
			SET_FG_PIXEL8_MASK(0); break; \
		case MINTERM_NEOR: \
			d ^= (s & mask); break; \
		case MINTERM_DST: /* This one does nothing. */ \
			return; break; \
		case MINTERM_NOTONLYSRC: \
			d |= (s & mask); break; \
		case MINTERM_SRC: \
			SET_FG_PIXEL8_MASK(0); break; \
		case MINTERM_NOTONLYDST: \
			s = ~(d & s) & mask; \
			SET_FG_PIXEL8_MASK(0); break; \
		case MINTERM_OR: \
			d |= (s & mask); break; \
	}

#define HANDLE_MINTERM_PIXEL_16_32(s, d) \
	switch (draw_mode) { \
		case MINTERM_NOR: \
			switch (color_format) { \
				case MNTVA_COLOR_16BIT565: \
					s &= ~(((uint16_t *)d)[x]); \
					SET_FG_PIXEL16_MASK(0); break; \
				case MNTVA_COLOR_32BIT: \
					s &= ~(d[x]); \
					SET_FG_PIXEL32_MASK(0); break; \
			} break; \
		case MINTERM_ONLYDST: \
			switch (color_format) { \
				case MNTVA_COLOR_16BIT565: \
					((uint16_t *)d)[x] = ((uint16_t *)d)[x] & ~(s); break; \
				case MNTVA_COLOR_32BIT: \
					d[x] = d[x] & ~(s); break; \
			} break; \
		case MINTERM_ONLYSRC: \
			switch (color_format) { \
				case MNTVA_COLOR_16BIT565: \
					s &= (((uint16_t *)d)[x] ^ 0xFFFF); \
					SET_FG_PIXEL16_MASK(0); break; \
				case MNTVA_COLOR_32BIT: \
					s &= (d[x] ^ 0x00FFFFFF); \
					SET_FG_PIXEL32_MASK(0); break; \
			} break; \
		case MINTERM_INVERT: \
			switch (color_format) { \
				case MNTVA_COLOR_16BIT565: \
					((uint16_t *)d)[x] ^= 0xFFFF; break; \
				case MNTVA_COLOR_32BIT: \
					d[x] ^= 0x00FFFFFF; break; \
			} break; \
		case MINTERM_EOR: \
			switch (color_format) { \
				case MNTVA_COLOR_16BIT565: \
					((uint16_t *)d)[x] ^= s; break; \
				case MNTVA_COLOR_32BIT: \
					d[x] ^= s; break; \
			} break; \
		case MINTERM_NAND: \
			switch (color_format) { \
				case MNTVA_COLOR_16BIT565: \
					s = ~(((uint16_t *)d)[x] | ~(s)) & color_mask; \
					SET_FG_PIXEL16_MASK(0); break; \
				case MNTVA_COLOR_32BIT: \
					s = ~(d[x] | ~(s)) & color_mask; \
					SET_FG_PIXEL32_MASK(0); break; \
			} break; \
		case MINTERM_AND: \
			switch (color_format) { \
				case MNTVA_COLOR_16BIT565: \
					s &= ((uint16_t *)d)[x]; \
					SET_FG_PIXEL16_MASK(0); break; \
				case MNTVA_COLOR_32BIT: \
					s &= d[x]; \
					SET_FG_PIXEL32_MASK(0); break; \
			} break; \
		case MINTERM_NEOR: \
			switch (color_format) { \
				case MNTVA_COLOR_16BIT565: \
					((uint16_t *)d)[x] ^= (s & color_mask); break; \
				case MNTVA_COLOR_32BIT: \
					d[x] ^= (s & color_mask); break; \
			} break; \
		case MINTERM_DST: /* This one does nothing. */ \
			return; break; \
		case MINTERM_NOTONLYSRC: \
			switch (color_format) { \
				case MNTVA_COLOR_16BIT565: \
					((uint16_t *)d)[x] |= (s & color_mask); break; \
				case MNTVA_COLOR_32BIT: \
					d[x] |= (s & color_mask); break; \
			} break; \
		case MINTERM_NOTSRC: \
		case MINTERM_SRC: \
			switch (color_format) { \
				case MNTVA_COLOR_16BIT565: \
					SET_FG_PIXEL16_MASK(0); break; \
				case MNTVA_COLOR_32BIT: \
					SET_FG_PIXEL32_MASK(0); break; \
			} break; \
		case MINTERM_NOTONLYDST: \
			switch (color_format) { \
				case MNTVA_COLOR_16BIT565: \
					((uint16_t *)d)[x] = ~(((uint16_t *)d)[x] & s) & color_mask; \
					SET_FG_PIXEL16_MASK(0); break; \
				case MNTVA_COLOR_32BIT: \
					d[x] = ~(d[x] & s) & color_mask; \
					SET_FG_PIXEL32_MASK(0); break; \
			} break; \
		case MINTERM_OR: \
			switch (color_format) { \
				case MNTVA_COLOR_16BIT565: \
					((uint16_t *)d)[x] |= (s & color_mask); break; \
				case MNTVA_COLOR_32BIT: \
					d[x] |= (s & color_mask); break; \
			} break; \
	}
