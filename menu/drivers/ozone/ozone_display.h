/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 *  Copyright (C) 2014-2017 - Jean-André Santoni
 *  Copyright (C) 2016-2017 - Brad Parker
 *  Copyright (C) 2018      - Alfredo Monclús
 *  Copyright (C) 2018      - natinusala
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _OZONE_DISPLAY_H
#define _OZONE_DISPLAY_H

#include "ozone.h"

#include "../../menu_driver.h"

typedef struct ozone_font
{
   font_data_t *font_data;
   video_font_raster_block_t raster_block;

   int glyph_width;
   int glyph_height;
}  ozone_font_t;

void ozone_font_init(ozone_font_t *font, char *path, unsigned size, bool is_threaded);

void ozone_font_free(ozone_font_t *font);

void ozone_font_bind(ozone_font_t *font);

void ozone_font_unbind(ozone_font_t *font);

void ozone_font_flush(ozone_font_t *font, video_frame_info_t *video_info);

void ozone_draw_text(
      video_frame_info_t *video_info,
      ozone_handle_t *ozone,
      const char *str, float x,
      float y,
      enum text_alignment text_align,
      ozone_font_t *font,
      uint32_t color,
      bool draw_outside);

void ozone_draw_cursor(ozone_handle_t *ozone,
      video_frame_info_t *video_info,
      int x_offset,
      unsigned width, unsigned height,
      size_t y, float alpha);

void ozone_color_alpha(float *color, float alpha);

void ozone_draw_icon(
      video_frame_info_t *video_info,
      unsigned icon_width,
      unsigned icon_height,
      uintptr_t texture,
      float x, float y,
      float rotation, float scale_factor,
      float *color);

void ozone_restart_cursor_animation(ozone_handle_t *ozone);

void ozone_draw_backdrop(video_frame_info_t *video_info, float alpha);

void ozone_draw_osk(ozone_handle_t *ozone,
      video_frame_info_t *video_info,
      const char *label, const char *str);

void ozone_draw_messagebox(ozone_handle_t *ozone,
      video_frame_info_t *video_info,
      const char *message);

void ozone_draw_quad(video_frame_info_t *video_info,
      int x, int y,
      unsigned width, unsigned height,
      float *color);

#endif