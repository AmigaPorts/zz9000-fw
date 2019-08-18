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
#include <string.h>
#include <math.h>
#include "gfx.h"

static uint32_t* fb=0;
static uint32_t fb_pitch=0;

void set_fb(uint32_t* fb_, uint32_t pitch) {
	fb=fb_;
	fb_pitch=pitch;
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
