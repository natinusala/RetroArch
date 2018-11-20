/*  RetroArch - A frontend for libretro.
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

#ifndef _OZONE_METRICS_H
#define _OZONE_METRICS_H

/* Baseline resolution is 1280x720 */
/* Those are all baseline metrics before scaling */

/* Font sizes */
#define FONT_SIZE_FOOTER            18
#define FONT_SIZE_TITLE             36
#define FONT_SIZE_TIME              22
#define FONT_SIZE_ENTRIES_LABEL     24
#define FONT_SIZE_ENTRIES_SUBLABEL  18
#define FONT_SIZE_SIDEBAR           24

/* Header metrics */
#define HEADER_HORIZONTAL_PADDING   47
#define HEADER_HEIGHT               87
#define HEADER_SEPARATOR_PADDING    30

#define HEADER_ICON_SIZE            60

#define HEADER_TIME_ICON_SIZE       92
#define HEADER_TIME_ICON_Y          2
#define HEADER_TIME_ICON_SPACING    15
#define HEADER_TIMEDATE_OFFSET      95

/* Entries metrics */
#define ENTRIES_VERTICAL_PADDING 40

/* Sidebar metrics */
#define SIDEBAR_VERTICAL_PADDING 30

/* Struct holding calculated and scaled metrics */
/* Stored in the handle */
typedef struct ozone_metrics
{
   struct {
      unsigned footer;
      unsigned title;
      unsigned time;
      unsigned entries_label;
      unsigned entries_sublabel;
      unsigned sidebar;
   } font;

   struct {
      unsigned horizontal_padding;
      unsigned height;
      unsigned separator_padding;

      unsigned icon_size;
      unsigned icon_y;

      unsigned title_y;

      unsigned time_icon_size;
      unsigned time_icon_y;
      unsigned time_icon_spacing;
      unsigned time_icon_offset;
      unsigned time_y;

      unsigned timedate_offset;
   } header;

   struct {
      unsigned start_y;
   } entries;

   struct {
      unsigned start_y;
   } sidebar;

   float scale_factor;
} ozone_metrics_t;

void ozone_compute_metrics(ozone_handle_t *ozone,
      float scale_factor);

#endif