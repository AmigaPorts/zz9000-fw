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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gfx.h"

static uint32_t* fb=0;
static uint32_t fb_pitch=0;

void set_fb(uint32_t* fb_, uint32_t pitch) {
	fb=fb_;
	fb_pitch=pitch;
}

extern uint32_t* sprite_buf;
void video_formatter_write(uint32_t data, uint16_t op);

void update_hw_sprite(uint8_t *data, uint32_t *colors, uint16_t w, uint16_t h)
{
	uint8_t cur_bit = 0x80;
	uint8_t cur_color = 0, out_pos = 0, iter_offset = 0;
	uint8_t line_pitch = (w / 8) * 2;
	uint8_t cur_bytes[8];

	for (uint8_t y_line = 0; y_line < h; y_line++) {
		if (w <= 16) {
			cur_bytes[0] = data[y_line * line_pitch];
			cur_bytes[1] = data[(y_line * line_pitch) + 2];
			cur_bytes[2] = data[(y_line * line_pitch) + 1];
			cur_bytes[3] = data[(y_line * line_pitch) + 3];
		}
		else {
			cur_bytes[0] = data[y_line * line_pitch];
			cur_bytes[1] = data[(y_line * line_pitch) + 4];
			cur_bytes[2] = data[(y_line * line_pitch) + 1];
			cur_bytes[3] = data[(y_line * line_pitch) + 5];
			cur_bytes[4] = data[(y_line * line_pitch) + 2];
			cur_bytes[5] = data[(y_line * line_pitch) + 6];
			cur_bytes[6] = data[(y_line * line_pitch) + 3];
			cur_bytes[7] = data[(y_line * line_pitch) + 7];
		}

		while(out_pos < 8) {
			for (uint8_t i = 0; i < line_pitch; i += 2) {
				cur_color = (cur_bytes[i] & cur_bit) ? 1 : 0;
				if (cur_bytes[i + 1] & cur_bit) cur_color += 2;

				sprite_buf[(y_line * 32) + out_pos + iter_offset] = colors[cur_color] & 0x00ffffff;

				video_formatter_write(((y_line * 32) + out_pos + iter_offset), 14);
				video_formatter_write(colors[cur_color] & 0x00ffffff, 15);
				iter_offset += 8;
			}

			out_pos++;
			cur_bit >>= 1;
			iter_offset = 0;
		}
		cur_bit = 0x80;
		out_pos = 0;
	}
}

void clip_hw_sprite(int16_t offset_x, int16_t offset_y)
{
	uint16_t xo = 0, yo = 0;
	if (offset_x < 0)
		xo = -offset_x;
	if (offset_y < 0)
		yo = -offset_y;

	for (int y = 0; y < 48; y++) {
		for (int x = 0; x < 32; x++) {
			video_formatter_write((y * 32) + x, 14);
			if (x < 32 - xo && y < 48 - yo)
				video_formatter_write(sprite_buf[((y + yo) * 32) + (x + xo)] & 0x00ffffff, 15);
			else
				video_formatter_write(0x00ff00ff, 15);
		}
	}
}

void clear_hw_sprite()
{
	for (uint16_t i = 0; i < 32 * 48; i++) {
		sprite_buf[i] = 0x00ff00ff;
		video_formatter_write(i, 14);
		video_formatter_write(0xff00ff, 15);
	}
}

void horizline(uint16_t x1, uint16_t x2, uint16_t y, uint32_t color) {
	uint32_t* p=fb+y*fb_pitch;
	uint16_t tmp;
	if (x2>x1) {
		tmp=x1; x1=x2; x2=tmp;
	}
	while (x1>x2) {
		p[x1--]=color;
	}
}

void fill_rect(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint32_t fg_color, uint32_t color_format, uint8_t mask)
{
	uint32_t* dp = fb + (rect_y1 * fb_pitch);
	uint8_t u8_fg = fg_color >> 24;
	uint16_t rect_y2 = rect_y1 + h, rect_x2 = rect_x1 + w;
	uint16_t x;

	for (uint16_t cur_y = rect_y1; cur_y < rect_y2; cur_y++) {
		x = rect_x1;
		switch(color_format) {
			case MNTVA_COLOR_8BIT:
				while(x < rect_x2) {
					SET_FG_PIXEL8_MASK(0);
					x++;
				}
				break;
			case MNTVA_COLOR_32BIT:
			case MNTVA_COLOR_16BIT565:
				while(x < rect_x2) {
					// The mask isn't used at all for 16/32-bit
					SET_FG_PIXEL;
					x++;
				}
				break;
			default:
				// Unknown/unhandled color format.
				break;
		}
		dp += fb_pitch;
	}
}

void fill_rect_solid(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint32_t rect_rgb, uint32_t color_format)
{
	uint32_t* p = fb + (rect_y1 * fb_pitch);
	uint16_t* p16;
	uint16_t rect_y2 = rect_y1 + h, rect_x2 = rect_x1 + w;
	uint16_t x;

	for (uint16_t cur_y = rect_y1; cur_y < rect_y2; cur_y++) {
		switch(color_format) {
			case MNTVA_COLOR_8BIT:
				memset((uint8_t *)p + rect_x1, (uint8_t)(rect_rgb >> 24), w);
				break;
			case MNTVA_COLOR_16BIT565:
				x = rect_x1;
				p16 = (uint16_t *)p;
				while(x < rect_x2) {
					p16[x++] = rect_rgb;
				}
				break;
			case MNTVA_COLOR_32BIT:
				x = rect_x1;
				while(x < rect_x2) {
					p[x++] = rect_rgb;
				}
				break;
			default:
				// Unknown/unhandled color format.
				break;
		}
		p += fb_pitch;
	}
}

void invert_rect(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint8_t mask, uint32_t color_format)
{
	uint32_t* dp = fb + (rect_y1 * fb_pitch);
	uint16_t x;

	uint16_t rect_y2 = rect_y1 + h, rect_x2 = rect_x1 + w;

	for (uint16_t cur_y = rect_y1; cur_y < rect_y2; cur_y++) {
		x = rect_x1;
		while (x < rect_x2) {
			INVERT_PIXEL;
			x++;
		}
		dp += fb_pitch;
	}
}

void fill_rect32(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint32_t rect_rgb) {
	for (uint16_t y=rect_y1; y<=rect_y2; y++) {
		uint32_t* p=fb+y*fb_pitch;
		for (uint16_t x=rect_x1; x<=rect_x2; x++) {
			p[x]=rect_rgb;
		}
	}
}

void fill_rect8(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint8_t rect_rgb) {
	for (uint16_t y=rect_y1; y<=rect_y2; y++) {
		uint8_t* p=(uint8_t*)(fb+y*fb_pitch);
		//for (uint16_t x=rect_x1; x<=rect_x2; x++) {
		//	p[x]=rect_rgb;
		//}
		memset(p+rect_x1,rect_rgb,rect_x2-rect_x1+1);
	}
}

void fill_rect16(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint16_t rect_rgb) {
	for (uint16_t y=rect_y1; y<=rect_y2; y++) {
		uint16_t* p=(uint16_t*)(fb+y*fb_pitch);
		for (uint16_t x=rect_x1; x<=rect_x2; x++) {
			p[x]=rect_rgb;
		}
	}
}

void copy_rect_nomask(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint16_t rect_sx, uint16_t rect_sy, uint32_t color_format, uint32_t* sp_src, uint32_t src_pitch, uint8_t draw_mode)
{
	uint32_t* dp = fb + (rect_y1 * fb_pitch);
	uint32_t* sp = sp_src + (rect_sy * src_pitch);
	uint16_t rect_y2 = rect_y1 + h - 1;
	uint8_t mask = 0xFF; // Perform mask handling, just in case we get a FillRectComplete at some point.
	uint32_t color_mask = 0x00FFFFFF;

	uint8_t u8_fg = 0;
	uint32_t fg_color = 0;

	int32_t line_step_d = fb_pitch, line_step_s = src_pitch;
	int8_t x_reverse = 0;

	if (rect_sy < rect_y1) {
		line_step_d = -fb_pitch;
		dp = fb + (rect_y2 * fb_pitch);
		line_step_s = -src_pitch;
		sp = sp_src + ((rect_sy + h - 1) * src_pitch);
	}

	if (rect_sx < rect_x1) {
		x_reverse = 1;
	}

	if (draw_mode == MINTERM_SRC) {
		for (uint16_t y_line = 0; y_line < h; y_line++) {
			switch(color_format) {
				case MNTVA_COLOR_8BIT:
					if (!x_reverse)
						memcpy((uint8_t *)dp + rect_x1, (uint8_t *)sp + rect_sx, w);
					else
						memmove((uint8_t *)dp + rect_x1, (uint8_t *)sp + rect_sx, w);
					break;
				case MNTVA_COLOR_16BIT565:
					if (!x_reverse)
						memcpy((uint16_t *)dp + rect_x1, (uint16_t *)sp + rect_sx, w * 2);
					else
						memmove((uint16_t *)dp + rect_x1, (uint16_t *)sp + rect_sx, w * 2);
					break;
				case MNTVA_COLOR_32BIT:
					if (!x_reverse)
						memcpy(dp + rect_x1, sp + rect_sx, w * 4);
					else
						memmove(dp + rect_x1, sp + rect_sx, w * 4);
					break;
			}
			dp += line_step_d;
			sp += line_step_s;
		}
	}
	else {
		for (uint16_t y_line = 0; y_line < h; y_line++) {
			if (x_reverse) {
				for (int16_t x = w; x >= 0; x--) {
					if (color_format == MNTVA_COLOR_8BIT) {
						u8_fg = ((uint8_t *)sp)[rect_sx + x];
						HANDLE_MINTERM_PIXEL_8(u8_fg, ((uint8_t *)dp)[rect_x1 + x]);
					}
					else {
						if (color_format == MNTVA_COLOR_16BIT565)
							fg_color = ((uint16_t *)sp)[rect_sx + x];
						else
							fg_color = sp[rect_sx + x];
						HANDLE_MINTERM_PIXEL_16_32(fg_color, dp);
					}
				}
			}
			else {
				for (int16_t x = 0; x < w; x++) {
					if (color_format == MNTVA_COLOR_8BIT) {
						u8_fg = ((uint8_t *)sp)[rect_sx + x];
						HANDLE_MINTERM_PIXEL_8(u8_fg, ((uint8_t *)dp)[rect_x1 + x]);
					}
					else {
						if (color_format == MNTVA_COLOR_16BIT565)
							fg_color = ((uint16_t *)sp)[rect_sx + x];
						else
							fg_color = sp[rect_sx + x];
						HANDLE_MINTERM_PIXEL_16_32(fg_color, dp);
					}
				}
			}
			dp += line_step_d;
			sp += line_step_s;
		}
	}
}

void copy_rect(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint16_t rect_sx, uint16_t rect_sy, uint32_t color_format, uint32_t* sp_src, uint32_t src_pitch, uint8_t mask)
{
	uint32_t* dp = fb + (rect_y1 * fb_pitch);
	uint32_t* sp = sp_src + (rect_sy * src_pitch);
	uint16_t rect_y2 = rect_y1 + h - 1;//, rect_x2 = rect_x1 + h - 1;

	int32_t line_step_d = fb_pitch, line_step_s = src_pitch;
	int8_t x_reverse = 0;

	if (rect_sy < rect_y1) {
		line_step_d = -fb_pitch;
		dp = fb + (rect_y2 * fb_pitch);
		line_step_s = -src_pitch;
		sp = sp_src + ((rect_sy + h - 1) * src_pitch);
	}

	if (rect_sx < rect_x1) {
		x_reverse = 1;
	}

	for (uint16_t y_line = 0; y_line < h; y_line++) {
		if (x_reverse) {
			for (int16_t x = w; x >= 0; x--) {
				((uint8_t *)dp)[rect_x1 + x] = (((uint8_t *)dp)[rect_x1 + x] & (mask ^ 0xFF)) | (((uint8_t *)sp)[rect_sx + x] & mask);
			}
		}
		else {
			for (int16_t x = 0; x < w; x++) {
				((uint8_t *)dp)[rect_x1 + x] = (((uint8_t *)dp)[rect_x1 + x] & (mask ^ 0xFF)) | (((uint8_t *)sp)[rect_sx + x] & mask);
			}
		}
		dp += line_step_d;
		sp += line_step_s;
	}
}

void copy_rect32(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint16_t rect_sx, uint16_t rect_sy) {
	int8_t ystep=1, xstep=1;
	uint16_t tmp;
	if (rect_sy < rect_y1) {
		uint16_t h = rect_y2-rect_y1;
		ystep=-1;
		tmp=rect_y2; rect_y2=rect_y1; rect_y1=tmp;
		rect_sy+=h;
	}
	if (rect_sx < rect_x1) {
		uint16_t w = rect_x2-rect_x1;
		xstep=-1;
		tmp=rect_x2; rect_x2=rect_x1; rect_x1=tmp;
		rect_sx+=w;
	}
	rect_y2+=ystep;
	rect_x2+=xstep;
	for (uint16_t sy=rect_sy, dy=rect_y1; dy!=rect_y2; sy+=ystep, dy+=ystep) {
		uint32_t* dp=(uint32_t*)(fb+dy*fb_pitch);
		uint32_t* sp=(uint32_t*)(fb+sy*fb_pitch);
		for (uint16_t sx=rect_sx, dx=rect_x1; dx!=rect_x2; sx+=xstep, dx+=xstep) {
			dp[dx]=sp[sx];
		}
	}
}

void copy_rect16(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint16_t rect_sx, uint16_t rect_sy) {
	int8_t ystep=1, xstep=1;
	uint16_t tmp;
	if (rect_sy < rect_y1) {
		uint16_t h = rect_y2-rect_y1;
		ystep=-1;
		tmp=rect_y2; rect_y2=rect_y1; rect_y1=tmp;
		rect_sy+=h;
	}
	if (rect_sx < rect_x1) {
		uint16_t w = rect_x2-rect_x1;
		xstep=-1;
		tmp=rect_x2; rect_x2=rect_x1; rect_x1=tmp;
		rect_sx+=w;
	}
	rect_y2+=ystep;
	rect_x2+=xstep;
	for (uint16_t sy=rect_sy, dy=rect_y1; dy!=rect_y2; sy+=ystep, dy+=ystep) {
		uint16_t* dp=(uint16_t*)(fb+dy*fb_pitch);
		uint16_t* sp=(uint16_t*)(fb+sy*fb_pitch);
		for (uint16_t sx=rect_sx, dx=rect_x1; dx!=rect_x2; sx+=xstep, dx+=xstep) {
			dp[dx]=sp[sx];
		}
	}
}

void copy_rect8(uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2, uint16_t rect_sx, uint16_t rect_sy) {
	int8_t ystep=1, xstep=1;
	uint16_t tmp;
	if (rect_sy < rect_y1) {
		uint16_t h = rect_y2-rect_y1;
		ystep=-1;
		tmp=rect_y2; rect_y2=rect_y1; rect_y1=tmp;
		rect_sy+=h;
	}
	if (rect_sx < rect_x1) {
		uint16_t w = rect_x2-rect_x1;
		xstep=-1;
		tmp=rect_x2; rect_x2=rect_x1; rect_x1=tmp;
		rect_sx+=w;
	}
	rect_y2+=ystep;
	rect_x2+=xstep;
	for (uint16_t sy=rect_sy, dy=rect_y1; dy!=rect_y2; sy+=ystep, dy+=ystep) {
		uint8_t* dp=(uint8_t*)(fb+dy*fb_pitch);
		uint8_t* sp=(uint8_t*)(fb+sy*fb_pitch);
		for (uint16_t sx=rect_sx, dx=rect_x1; dx!=rect_x2; sx+=xstep, dx+=xstep) {
			dp[dx]=sp[sx];
		}
	}
}

#define DRAW_LINE_PIXEL \
	if (draw_mode == JAM1) { \
		if(pattern & cur_bit) { \
			if (!inversion) { \
				if (mask == 0xFF || color_format == MNTVA_COLOR_16BIT565 || color_format == MNTVA_COLOR_32BIT) { SET_FG_PIXEL; } \
				else { SET_FG_PIXEL8_MASK(0) } \
			} \
			else { INVERT_PIXEL; } \
		} \
	} \
	else { \
		if(pattern & cur_bit) { \
			if (!inversion) { \
				if (mask == 0xFF || color_format == MNTVA_COLOR_16BIT565 || color_format == MNTVA_COLOR_32BIT) { SET_FG_PIXEL; } \
				else { SET_FG_PIXEL8_MASK(0); } \
			} \
			else { INVERT_PIXEL; } /* JAM2 and complement is kind of useless, as it ends up being the same visual result as JAM1 and a pattern of 0xFFFF */ \
		} \
		else { \
			if (!inversion) { \
				if (mask == 0xFF || color_format == MNTVA_COLOR_16BIT565 || color_format == MNTVA_COLOR_32BIT) { SET_BG_PIXEL; } \
				else { SET_BG_PIXEL8_MASK(0); } \
			} \
			else { INVERT_PIXEL; } \
		} \
	} \
	if ((cur_bit >>= 1) == 0) \
		cur_bit = 0x8000; \

// Sneakily adapted version of the good old Bresenham algorithm
void draw_line(int16_t rect_x1, int16_t rect_y1, int16_t rect_x2, int16_t rect_y2, uint16_t len,
	uint16_t pattern, uint16_t pattern_offset,
	uint32_t fg_color, uint32_t bg_color, uint32_t color_format,
	uint8_t mask, uint8_t draw_mode)
{
	int16_t x1 = rect_x1, y1 = rect_y1;
	int16_t x2 = rect_x1 + rect_x2, y2 = rect_y1 + rect_y2;

	uint8_t u8_fg = fg_color >> 24;
	uint8_t u8_bg = bg_color >> 24;

	uint32_t* dp = fb + (y1 * fb_pitch);
	int32_t line_step = fb_pitch;
	int8_t x_reverse = 0, inversion = 0;

	uint16_t cur_bit = 0x8000;

	int16_t dx, dy, dx_abs, dy_abs, ix, iy, x = x1;

	if (x2 < x1)
		x_reverse = 1;
	if (y2 < y1)
		line_step = -fb_pitch;

	if (draw_mode & INVERSVID)
		pattern ^= 0xFFFF;
	if (draw_mode & COMPLEMENT) {
		inversion = 1;
		fg_color = 0xFFFF0000;
	}
	draw_mode &= 0x01;

	dx = x2 - x1;
	dy = y2 - y1;
	dx_abs = abs(dx);
	dy_abs = abs(dy);
	ix = dy_abs >> 1;
	iy = dx_abs >> 1;

	// This can't be used for now, as Flags from the current RastPort struct is not exposed by [ P96 2.4.2 ]
	/*if ((pattern_offset >> 8) & 0x01) { // Is FRST_DOT set?
		cur_bit = 0x8000;
		fg_color = 0xFFFF0000;
	}
	else {
		fg_color = 0xFF00FF00;
		cur_bit >>= ((pattern_offset & 0xFF) % 16);
	}
	
	if (cur_bit == 0)
		cur_bit = 0x8000;*/


	DRAW_LINE_PIXEL;

	if (dx_abs >= dy_abs) {
		if (!len) len = dx_abs;
		for (uint16_t i = 0; i < len; i++) {
			iy += dy_abs;
			if (iy >= dx_abs) {
				iy -= dx_abs;
				dp += line_step;
			}
			x += (x_reverse) ? -1 : 1;

			DRAW_LINE_PIXEL;
		}
	}
	else {
		if (!len) len = dy_abs;
		for(uint16_t i = 0; i < len; i++) {
			ix += dx_abs;
			if (ix >= dy_abs) {
				ix -= dy_abs;
				x += (x_reverse) ? -1 : 1;
			}
			dp += line_step;

			DRAW_LINE_PIXEL;
		}
	}
}

void draw_line_solid(int16_t rect_x1, int16_t rect_y1, int16_t rect_x2, int16_t rect_y2, uint16_t len,
	uint32_t fg_color, uint32_t color_format)
{
	int16_t x1 = rect_x1, y1 = rect_y1;
	int16_t x2 = rect_x1 + rect_x2, y2 = rect_y1 + rect_y2;

	uint8_t u8_fg = fg_color >> 24;

	uint32_t* dp = fb + (y1 * fb_pitch);
	int32_t line_step = fb_pitch;
	int8_t x_reverse = 0;

	int16_t dx, dy, dx_abs, dy_abs, ix, iy, x = x1;

	if (x2 < x1)
		x_reverse = 1;
	if (y2 < y1)
		line_step = -fb_pitch;

	dx = x2 - x1;
	dy = y2 - y1;
	dx_abs = abs(dx);
	dy_abs = abs(dy);
	ix = dy_abs >> 1;
	iy = dx_abs >> 1;

	SET_FG_PIXEL;

	if (dx_abs >= dy_abs) {
		if (!len) len = dx_abs;
		for (uint16_t i = 0; i < len; i++) {
			iy += dy_abs;
			if (iy >= dx_abs) {
				iy -= dx_abs;
				dp += line_step;
			}
			x += (x_reverse) ? -1 : 1;

			SET_FG_PIXEL;
		}
	}
	else {
		if (!len) len = dy_abs;
		for (uint16_t i = 0; i < len; i++) {
			ix += dx_abs;
			if (ix >= dy_abs) {
				ix -= dy_abs;
				x += (x_reverse) ? -1 : 1;
			}
			dp += line_step;

			SET_FG_PIXEL;
		}
	}
}

#define DECODE_PLANAR_PIXEL(a) \
	switch (planes) { \
		case 8: if (layer_mask & 0x80 && bmp_data[(plane_size * 7) + cur_byte] & cur_bit) a |= 0x80; \
		case 7: if (layer_mask & 0x40 && bmp_data[(plane_size * 6) + cur_byte] & cur_bit) a |= 0x40; \
		case 6: if (layer_mask & 0x20 && bmp_data[(plane_size * 5) + cur_byte] & cur_bit) a |= 0x20; \
		case 5: if (layer_mask & 0x10 && bmp_data[(plane_size * 4) + cur_byte] & cur_bit) a |= 0x10; \
		case 4: if (layer_mask & 0x08 && bmp_data[(plane_size * 3) + cur_byte] & cur_bit) a |= 0x08; \
		case 3: if (layer_mask & 0x04 && bmp_data[(plane_size * 2) + cur_byte] & cur_bit) a |= 0x04; \
		case 2: if (layer_mask & 0x02 && bmp_data[plane_size + cur_byte] & cur_bit) a |= 0x02; \
		case 1: if (layer_mask & 0x01 && bmp_data[cur_byte] & cur_bit) a |= 0x01; \
			break; \
	}

#define DECODE_INVERTED_PLANAR_PIXEL(a) \
	switch (planes) { \
		case 8: if (layer_mask & 0x80 && (bmp_data[(plane_size * 7) + cur_byte] ^ 0xFF) & cur_bit) a |= 0x80; \
		case 7: if (layer_mask & 0x40 && (bmp_data[(plane_size * 6) + cur_byte] ^ 0xFF) & cur_bit) a |= 0x40; \
		case 6: if (layer_mask & 0x20 && (bmp_data[(plane_size * 5) + cur_byte] ^ 0xFF) & cur_bit) a |= 0x20; \
		case 5: if (layer_mask & 0x10 && (bmp_data[(plane_size * 4) + cur_byte] ^ 0xFF) & cur_bit) a |= 0x10; \
		case 4: if (layer_mask & 0x08 && (bmp_data[(plane_size * 3) + cur_byte] ^ 0xFF) & cur_bit) a |= 0x08; \
		case 3: if (layer_mask & 0x04 && (bmp_data[(plane_size * 2) + cur_byte] ^ 0xFF) & cur_bit) a |= 0x04; \
		case 2: if (layer_mask & 0x02 && (bmp_data[plane_size + cur_byte] ^ 0xFF) & cur_bit) a |= 0x02; \
		case 1: if (layer_mask & 0x01 && (bmp_data[cur_byte] ^ 0xFF) & cur_bit) a |= 0x01; \
			break; \
	}

void p2c_rect(int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t w, int16_t h, uint16_t sh, uint8_t draw_mode, uint8_t planes, uint8_t mask, uint8_t layer_mask, uint16_t src_line_pitch, uint8_t *bmp_data_src)
{
	uint32_t *dp = fb + (dy * fb_pitch);

	uint8_t cur_bit, base_bit, base_byte;
	uint16_t cur_byte = 0, u8_fg = 0;

	uint32_t plane_size = src_line_pitch * h;
	uint8_t *bmp_data = bmp_data_src;

	cur_bit = base_bit = (0x80 >> (sx % 8));
	cur_byte = base_byte = ((sx / 8) % src_line_pitch);

	for (int16_t line_y = 0; line_y < h; line_y++) {
		for (int16_t x = dx; x < dx + w; x++) {
			u8_fg = 0;
			if (draw_mode & 0x01) // If bit 1 is set, the inverted planar data is always used.
				DECODE_INVERTED_PLANAR_PIXEL(u8_fg)
			else
				DECODE_PLANAR_PIXEL(u8_fg)
			
			if (mask == 0xFF && (draw_mode == MINTERM_SRC || draw_mode == MINTERM_NOTSRC)) {
				((uint8_t *)dp)[x] = u8_fg;
				goto skip;
			}

			HANDLE_MINTERM_PIXEL_8(u8_fg, ((uint8_t *)dp)[x]);

			skip:;
			if ((cur_bit >>= 1) == 0) {
				cur_bit = 0x80;
				cur_byte++;
				cur_byte %= src_line_pitch;
			}

		}
		dp += fb_pitch;
		if ((line_y + sy + 1) % h)
			bmp_data += src_line_pitch;
		else
			bmp_data = bmp_data_src;
		cur_bit = base_bit;
		cur_byte = base_byte;
	}
}

void p2d_rect(int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t w, int16_t h, uint16_t sh, uint8_t draw_mode, uint8_t planes, uint8_t mask, uint8_t layer_mask, uint32_t color_mask, uint16_t src_line_pitch, uint8_t *bmp_data_src, uint32_t color_format)
{
	uint32_t *dp = fb + (dy * fb_pitch);

	uint8_t cur_bit, base_bit, base_byte;
	uint16_t cur_byte = 0, cur_pixel = 0;
	uint32_t fg_color = 0;

	uint32_t plane_size = src_line_pitch * h;
	uint32_t *bmp_pal = (uint32_t *)bmp_data_src;
	uint8_t *bmp_data = bmp_data_src + (256 * 4);

	cur_bit = base_bit = (0x80 >> (sx % 8));
	cur_byte = base_byte = ((sx / 8) % src_line_pitch);

	for (int16_t line_y = 0; line_y < h; line_y++) {
		for (int16_t x = dx; x < dx + w; x++) {
			cur_pixel = 0;
			if (draw_mode & 0x01)
				DECODE_INVERTED_PLANAR_PIXEL(cur_pixel)
			else
				DECODE_PLANAR_PIXEL(cur_pixel)
			fg_color = bmp_pal[cur_pixel];

			if (mask == 0xFF && (draw_mode == 0x0C || draw_mode == 0x03)) {
				switch (color_format) {
					case MNTVA_COLOR_16BIT565:
						((uint16_t *)dp)[x] = fg_color;
						break;
					case MNTVA_COLOR_32BIT:
						dp[x] = fg_color;
						break;
				}
				goto skip;
			}

			HANDLE_MINTERM_PIXEL_16_32(fg_color, dp);

			skip:;
			if ((cur_bit >>= 1) == 0) {
				cur_bit = 0x80;
				cur_byte++;
				cur_byte %= src_line_pitch;
			}

		}
		dp += fb_pitch;
		if ((line_y + sy + 1) % h)
			bmp_data += src_line_pitch;
		else
			bmp_data = bmp_data_src;
		cur_bit = base_bit;
		cur_byte = base_byte;
	}
}

#define PATTERN_FILLRECT_LOOPX \
	tmpl_x++; \
	if (loop_rows) \
		tmpl_x = tmpl_x % 2; \
	cur_byte = (inversion) ? tmpl_data[tmpl_x] ^ 0xFF : tmpl_data[tmpl_x];

#define PATTERN_FILLRECT_LOOPY \
	tmpl_data += (loop_rows > 0) ? 2 : tmpl_pitch; \
	if (loop_rows && (y_line + y_offset + 1) % loop_rows == 0) \
		tmpl_data = tmpl_base; \
	tmpl_x = tmpl_x_base; \
	cur_bit = base_bit; \
	dp += fb_pitch / 4;

void pattern_fill_rect(uint32_t color_format, uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h,
	uint8_t draw_mode, uint8_t mask, uint32_t fg_color, uint32_t bg_color,
	uint16_t x_offset, uint16_t y_offset,
	uint8_t *tmpl_data, uint16_t tmpl_pitch, uint16_t loop_rows)
{
	uint32_t rect_x2 = rect_x1 + w;
	uint32_t *dp = fb + (rect_y1 * (fb_pitch / 4));
	uint8_t* tmpl_base = tmpl_data;

	uint16_t tmpl_x, tmpl_x_base;

	uint8_t cur_bit, base_bit, inversion = 0;
	uint8_t u8_fg = fg_color >> 24;
	uint8_t u8_bg = bg_color >> 24;
	uint8_t cur_byte = 0;

	tmpl_x = x_offset / 8;
	if (loop_rows) {
		tmpl_x %= 2;
		tmpl_data += (y_offset % loop_rows) * 2;
	}
	tmpl_x_base = tmpl_x;

	cur_bit = base_bit = (0x80 >> (x_offset % 8));

	if (draw_mode & INVERSVID) inversion = 1;
	draw_mode &= 0x03;

	if (draw_mode == JAM1) {
		for (uint16_t y_line = 0; y_line < h; y_line++) {
			uint16_t x = rect_x1;

			cur_byte = (inversion) ? tmpl_data[tmpl_x] ^ 0xFF : tmpl_data[tmpl_x];

			while (x < rect_x2) {
				if (cur_bit == 0x80 && x < rect_x2 - 8) {
					if (mask == 0xFF) {
						SET_FG_PIXELS;
					}
					else {
						SET_FG_PIXELS_MASK;
					}
					x += 8;
				}
				else {
					while (cur_bit > 0 && x < rect_x2) {
						if (cur_byte & cur_bit) {
							SET_FG_PIXEL_MASK;
						}
						x++;
						cur_bit >>= 1;
					}
					cur_bit = 0x80;
				}
				PATTERN_FILLRECT_LOOPX;
			}
			PATTERN_FILLRECT_LOOPY;
		}

		return;
	}
	else if (draw_mode == JAM2) {
		for (uint16_t y_line = 0; y_line < h; y_line++) {
			uint16_t x = rect_x1;

			cur_byte = (inversion) ? tmpl_data[tmpl_x] ^ 0xFF : tmpl_data[tmpl_x];

			while (x < rect_x2) {
				if (cur_bit == 0x80 && x < rect_x2 - 8) {
					if (mask == 0xFF) {
						SET_FG_OR_BG_PIXELS;
					}
					else {
						SET_FG_OR_BG_PIXELS_MASK;
					}
					x += 8;
				}
				else {
					while (cur_bit > 0 && x < rect_x2) {
						if (cur_byte & cur_bit) {
							SET_FG_PIXEL_MASK;
						}
						else {
							SET_BG_PIXEL_MASK;
						}
						x++;
						cur_bit >>= 1;
					}
					cur_bit = 0x80;
				}
				PATTERN_FILLRECT_LOOPX;
			}
			PATTERN_FILLRECT_LOOPY;
		}

		return;
	}
	else { // COMPLEMENT
		for (uint16_t y_line = 0; y_line < h; y_line++) {
			uint16_t x = rect_x1;

			cur_byte = (inversion) ? tmpl_data[tmpl_x] ^ 0xFF : tmpl_data[tmpl_x];

			while (x < rect_x2) {
				if (cur_bit == 0x80 && x < rect_x2 - 8) {
					INVERT_PIXELS;
					x += 8;
				}
				else {
					while (cur_bit > 0 && x < rect_x2) {
						if (cur_byte & cur_bit) {
							INVERT_PIXEL;
						}
						x++;
						cur_bit >>= 1;
					}
					cur_bit = 0x80;
				}
				PATTERN_FILLRECT_LOOPX;
			}
			PATTERN_FILLRECT_LOOPY;	
		}
	}
}

void fill_template(uint32_t bpp, uint16_t rect_x1, uint16_t rect_y1, uint16_t rect_x2, uint16_t rect_y2,
		uint8_t draw_mode, uint8_t mask, uint32_t fg_color, uint32_t bg_color, uint16_t x_offset, uint16_t y_offset, uint8_t* tmpl_data, uint16_t tmpl_pitch, uint16_t loop_rows)
{
	uint8_t inversion = 0;
	uint16_t rows;
	int bitoffset;
	uint8_t* dp=(uint8_t*)(fb);
	uint8_t* tmpl_base;

	uint16_t width = rect_x2-rect_x1+1;

	if (draw_mode & INVERSVID) inversion = 1;
	draw_mode &= 0x03;

	bitoffset = x_offset % 8;
	tmpl_base = tmpl_data + x_offset / 8;

	// starting position in destination
	dp += rect_y1*fb_pitch + rect_x1*bpp;

	// number of 8-bit blocks of source
	uint16_t loop_x = x_offset;
	uint16_t loop_y = y_offset;

	for (rows = rect_y1; rows <= rect_y2; rows++, dp += fb_pitch, tmpl_base += tmpl_pitch) {
		unsigned long cols;
		uint8_t* dp2 = dp;
		uint8_t* tmpl_mem;
		unsigned int data;

		tmpl_mem = tmpl_base;
		data = *tmpl_mem;

		for (cols = 0; cols < width; cols += 8, dp2 += bpp*8) {
			unsigned int byte;
			long bits;
			long max = width - cols;

			if (max > 8) max = 8;

			// loop through 16-bit horizontal pattern
			if (loop_rows>0) {
				tmpl_mem = tmpl_data+(loop_y%loop_rows)*2;
				byte = tmpl_mem[loop_x%2];
				loop_x++;
			} else {
				data <<= 8;
				data |= *++tmpl_mem;
				byte = data >> (8 - bitoffset);
			}

			switch (draw_mode)
			{
				case JAM1:
				{
					for (bits = 0; bits < max; bits++) {
						int bit_set = (byte & 0x80);
						byte <<= 1;
						if (inversion) bit_set = !bit_set;

						if (bit_set) {

							if (bpp == 1) {
								dp2[bits] = fg_color>>24;
							} else if (bpp == 2) {
								((uint16_t*)dp2)[bits] = fg_color;
							} else if (bpp == 4) {
								((uint32_t*)dp2)[bits] = fg_color;
							}

							// TODO mask
							//dp2[bits] = (fg_color & mask) | (dp2[bits] & ~mask);
						}
					}
					break;
				}
				case JAM2:
				{
					for (bits = 0; bits < max; bits++) {
						char bit_set = (byte & 0x80);
						byte <<= 1;
						if (inversion) bit_set = !bit_set;

						uint32_t color = bit_set ? fg_color : bg_color;
						//if (bit_set) printf("#");
						//else printf(".");

						if (bpp == 1) {
							dp2[bits] = color>>24;
						} else if (bpp == 2) {
							((uint16_t*)dp2)[bits] = color;
						} else if (bpp == 4) {
							((uint32_t*)dp2)[bits] = color;
						}

						// NYI
						//	dp2[bits] = (color & mask) | (dp2[bits] & ~mask);
					}
					break;
				}
				case COMPLEMENT:
				{
					for (bits = 0; bits < max; bits++) {
						int bit_set = (byte & 0x80);
						byte <<= 1;
						if (bit_set) {
							if (bpp == 1) {
								dp2[bits] ^= 0xff;
							} else if (bpp == 2) {
								((uint16_t*)dp2)[bits] ^= 0xffff;
							} else if (bpp == 4) {
								((uint32_t*)dp2)[bits] ^= 0xffffffff;
							}
						}
						// TODO mask
					}
					break;
				}
			}
		}
		loop_y++;
	}
}

