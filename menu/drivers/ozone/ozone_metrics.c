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

#include "ozone.h"
#include "ozone_metrics.h"

/* TODO Find a solution for images scaling looking bad (downscale them instead of upscaling them) */

void ozone_compute_metrics(ozone_handle_t *ozone,
      float scale_factor)
{
   ozone_metrics_t *metrics   = &ozone->metrics;

   /* Fonts */
   /* TODO Actually use those instead of the hardcoded defines in the whole code */
   metrics->font.footer             = FONT_SIZE_FOOTER            * scale_factor;
   metrics->font.title              = FONT_SIZE_TITLE             * scale_factor;
   metrics->font.time               = FONT_SIZE_TIME              * scale_factor;
   metrics->font.entries_label      = FONT_SIZE_ENTRIES_LABEL     * scale_factor;
   metrics->font.entries_sublabel   = FONT_SIZE_ENTRIES_SUBLABEL  * scale_factor;
   metrics->font.sidebar            = FONT_SIZE_SIDEBAR           * scale_factor;

   /* Header */
   metrics->header.horizontal_padding  = HEADER_HORIZONTAL_PADDING   * scale_factor;
   metrics->header.height              = HEADER_HEIGHT               * scale_factor;
   metrics->header.separator_padding   = HEADER_SEPARATOR_PADDING    * scale_factor;

   metrics->header.icon_size           = HEADER_ICON_SIZE                        * scale_factor;
   metrics->header.icon_y              = ((HEADER_HEIGHT - HEADER_ICON_SIZE)/2)  * scale_factor;
   
   metrics->header.title_y             = ((HEADER_HEIGHT - FONT_SIZE_TITLE)/2.5)   * scale_factor;

   metrics->header.time_icon_size      = HEADER_TIME_ICON_SIZE    * scale_factor;
   metrics->header.time_icon_y         = HEADER_TIME_ICON_Y       * scale_factor;
   metrics->header.time_icon_spacing   = HEADER_TIME_ICON_SPACING * scale_factor;

   metrics->header.time_icon_offset    = metrics->header.horizontal_padding*2 + metrics->header.time_icon_size/4;
   metrics->header.time_y              = ((HEADER_HEIGHT - FONT_SIZE_TIME)/2.2)   * scale_factor;
   metrics->header.timedate_offset     = HEADER_TIMEDATE_OFFSET                   * scale_factor;
  
   /* Entries */
   metrics->entries.start_y = (HEADER_HEIGHT + ENTRIES_VERTICAL_PADDING) * scale_factor;

   /* Sidebar */
   metrics->sidebar.start_y = (HEADER_HEIGHT + SIDEBAR_VERTICAL_PADDING) * scale_factor;

   /* Scale factor */
   metrics->scale_factor = scale_factor;
}