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

#ifndef _GFX_WIDGETS_H
#define _GFX_WIDGETS_H

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "font_driver.h"

#include <formats/image.h>
#include <queues/task_queue.h>
#include <queues/message_queue.h>

#define DEFAULT_BACKDROP               0.75f

#define MSG_QUEUE_PENDING_MAX          32
#define MSG_QUEUE_ONSCREEN_MAX         4

#define MSG_QUEUE_ANIMATION_DURATION      330
#define VOLUME_DURATION                   3000
#define SCREENSHOT_DURATION_IN            66
#define SCREENSHOT_DURATION_OUT           SCREENSHOT_DURATION_IN*10
#define SCREENSHOT_NOTIFICATION_DURATION  6000
#define CHEEVO_NOTIFICATION_DURATION      4000
#define TASK_FINISHED_DURATION            3000
#define HOURGLASS_INTERVAL                5000
#define HOURGLASS_DURATION                1000
#define GENERIC_MESSAGE_DURATION          3000

bool gfx_widgets_init(bool video_is_threaded, bool fullscreen);

void gfx_widgets_free(void);

void gfx_widgets_msg_queue_push(
      retro_task_t *task, const char *msg,
      unsigned duration,
      char *title,
      enum message_queue_icon icon,
      enum message_queue_category category,
      unsigned prio, bool flush,
      bool menu_is_alive);

void gfx_widgets_volume_update_and_show(float new_volume,
      bool mute);

void gfx_widgets_iterate(
      unsigned width, unsigned height, bool fullscreen,
      const char *dir_assets, char *font_path,
      bool is_threaded);

void gfx_widgets_screenshot_taken(const char *shotname, const char *filename);

/* AI Service functions */
#ifdef HAVE_TRANSLATE
int gfx_widgets_ai_service_overlay_get_state(void);
bool gfx_widgets_ai_service_overlay_set_state(int state);

bool gfx_widgets_ai_service_overlay_load(
        char* buffer, unsigned buffer_len,
        enum image_type_enum image_type);

void gfx_widgets_ai_service_overlay_unload(void);
#endif

void gfx_widgets_start_load_content_animation(
      const char *content_name, bool remove_extension);

void gfx_widgets_cleanup_load_content_animation(void);

void gfx_widgets_context_reset(bool is_threaded,
      unsigned width, unsigned height, bool fullscreen,
      const char *dir_assets, char *font_path);

void gfx_widgets_push_achievement(const char *title, const char *badge);

/* Warning: not thread safe! */
void gfx_widgets_set_message(char *message);

/* Warning: not thread safe! */
void gfx_widgets_set_libretro_message(const char *message, unsigned duration);

/* All the functions below should be called in
 * the video driver - once they are all added, set
 * enable_menu_widgets to true for that driver */
void gfx_widgets_frame(void *data);

bool gfx_widgets_set_fps_text(const char *new_fps_text);

font_data_t* gfx_widgets_get_font_regular();
font_data_t* gfx_widgets_get_font_bold();

void gfx_widgets_font_flush(font_data_t* font, video_frame_info_t* video_info);

#endif
