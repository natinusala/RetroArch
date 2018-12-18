/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2014-2017 - Jean-André Santoni
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

#include "menu_widgets.h"

#include "../../verbosity.h"
#include "../../retroarch.h"
#include "../../configuration.h"
#include "../../msg_hash.h"

#include "../menu_driver.h"
#include "../menu_animation.h"

#include "../../gfx/font_driver.h"

#include <lists/file_list.h>
#include <queues/fifo_queue.h>
#include <file/file_path.h>
#include <streams/file_stream.h>
#include <formats/image.h>
#include <string/stdstring.h>

#define PI 3.14159265359f

/* TODO Check that video threaded doesn't break everything */
/* TODO Don't show rewind indicator if coming from netplay desync */

static float backdrop[16] =  {
   0.00, 0.00, 0.00, 0.75,
   0.00, 0.00, 0.00, 0.75,
   0.00, 0.00, 0.00, 0.75,
   0.00, 0.00, 0.00, 0.75,
};

static float msg_queue_background[16]  = COLOR_HEX_TO_FLOAT(0x3A3A3A, 1.0f);
static float msg_queue_info[16]        = COLOR_HEX_TO_FLOAT(0x12ACF8, 1.0f);

static float msg_queue_task_progress_1[16] = COLOR_HEX_TO_FLOAT(0x55AE99, 1.0f); /* Color of first progress bar in a task message */
static float msg_queue_task_progress_2[16] = COLOR_HEX_TO_FLOAT(0x388BBD, 1.0f); /* Color of second progress bar in a task message (for multiple tasks with same message) */

static float color_task_progress_bar[16] = COLOR_HEX_TO_FLOAT(0x22B14C, 1.0f);

static unsigned text_color_info        = 0xD8EEFFFF;
static unsigned text_color_success     = 0x22B14CFF;
static unsigned text_color_error       = 0xC23B22FF;
static unsigned text_color_faint       = 0x878787FF;

static float volume_bar_background[16]    = COLOR_HEX_TO_FLOAT(0x1A1A1A, 1.0f);
static float volume_bar_normal[16]        = COLOR_HEX_TO_FLOAT(0x198AC6, 1.0f);
static float volume_bar_loud[16]          = COLOR_HEX_TO_FLOAT(0xF5DD19, 1.0f);
static float volume_bar_loudest[16]       = COLOR_HEX_TO_FLOAT(0xC23B22, 1.0f);

static bool init                             = false;
static uint64_t menu_widgets_frame_count     = 0;
static menu_animation_ctx_tag generic_tag    = (uintptr_t) &init;

/* TODO Only use ozone font if font setting is unset */

/* Font data */
font_data_t *font_regular;
font_data_t *font_bold;

video_font_raster_block_t font_raster_regular;
video_font_raster_block_t font_raster_bold;

static float menu_widgets_pure_white[16] = {
      1.00, 1.00, 1.00, 1.00,
      1.00, 1.00, 1.00, 1.00,
      1.00, 1.00, 1.00, 1.00,
      1.00, 1.00, 1.00, 1.00,
};

/* Messages queue */

typedef struct menu_widget_msg
{
   char *msg;
   unsigned msg_len;
   unsigned duration;

   unsigned text_height;

   float offset_y;

   float alpha;

   bool dying; /* is it currently doing the fade out animation ? */

   unsigned width;
   bool expired; /* has the timer expired ? if so, should be set to dying */

   menu_timer_t expiration_timer;
   bool expiration_timer_started;

   retro_task_t *task_ptr;
   char *task_title_ptr; /* used to detect title change */

   int8_t task_progress;
   bool task_finished;
   bool task_error;
   bool task_cancelled;
   uint32_t task_ident;

   bool unfolded; /* unfold animation */
   bool unfolding;
   float unfold;

   float hourglass_rotation;
   menu_timer_t hourglass_timer;
} menu_widget_msg_t;

static fifo_buffer_t *msg_queue;
static file_list_t *current_msgs;
static file_list_t *task_msgs;
static unsigned msg_queue_kill;

/* TODO Don't display icons if assets are missing */

menu_texture_item hourglass_icon          = 0;
menu_texture_item info_icon               = 0;
menu_texture_item msg_queue_icon          = 0;
menu_texture_item msg_queue_icon_outline  = 0;
menu_texture_item msg_queue_icon_rect     = 0;

/* there can only be one message animation at a time to avoid confusing users */
static bool moving   = false;

/* Volume */
static float volume_db              = 0.0f;
static float volume_percent         = 1.0f;
static menu_timer_t volume_timer    = 0.0f;

static float volume_alpha                  = 0.0f;
static float volume_text_alpha             = 0.0f;
static menu_animation_ctx_tag volume_tag   = (uintptr_t) &volume_alpha;
static bool volume_mute                    = false;

static menu_texture_item volume_icon_med  = 0;
static menu_texture_item volume_icon_max  = 0;
static menu_texture_item volume_icon_min  = 0;
static menu_texture_item volume_icon_mute = 0;

/* FPS */
static char fps_text[255];

/* Status icons */
static bool paused         = false;
static bool fast_forward   = false;
static bool rewinding      = false;

static menu_texture_item paused_icon         = 0;
static menu_texture_item fast_forward_icon   = 0;
static menu_texture_item rewind_icon         = 0;
static menu_texture_item slowmotion_icon     = 0;

/* Screenshot */
static float screenshot_alpha                = 0.0f;
static menu_texture_item screenshot_texture  = 0;
static unsigned screenshot_texture_width     = 0;
static unsigned screenshot_texture_height    = 0;
static char screenshot_shotname[256]         = {0};
static char screenshot_filename[256]         = {0};
static bool screenshot_loaded                = false;

static unsigned screenshot_height;
static unsigned screenshot_width;
static float screenshot_scale_factor;
static unsigned screenshot_thumbnail_width;
static unsigned screenshot_thumbnail_height;
static float screenshot_y;
static menu_timer_t screenshot_timer;

static unsigned screenshot_shotname_length;

/* Metrics */
static unsigned simple_widget_padding;
static unsigned simple_widget_height;
static unsigned glyph_width;

static unsigned msg_queue_height;
static unsigned msg_queue_icon_size_x;
static unsigned msg_queue_icon_size_y;
static float msg_queue_text_scale_factor;
static unsigned msg_queue_base_width;
static unsigned msg_queue_spacing;
static unsigned msg_queue_glyph_width;
static unsigned msg_queue_rect_start_x;
static unsigned msg_queue_internal_icon_size;
static unsigned msg_queue_internal_icon_offset;
static unsigned msg_queue_icon_offset_y;
static unsigned msg_queue_scissor_start_x;
static unsigned msg_queue_default_rect_width;

bool menu_widgets_set_paused(bool is_paused)
{
   if (!init)
      return false;

   paused = is_paused;
   return true;
}

static bool menu_widgets_msg_queue_push_internal(retro_task_t *task, const char *msg,
      unsigned duration,
      char *title,
      enum message_queue_icon icon, enum message_queue_category category)
{
   menu_widget_msg_t* msg_widget = NULL;
   int i;

   if (!init)
      return false;

   #ifdef HAVE_THREADS
   runloop_msg_queue_lock();
   #endif

   if (fifo_write_avail(msg_queue) > 0)
   {
      /* Get current msg if it exists */
      if (task != NULL && task->frontend_userdata)
      {
         msg_widget = (menu_widget_msg_t*) task->frontend_userdata;
         msg_widget->task_ptr = task; /* msg_widgets can be passed between tasks */
      }

      /* Spawn a new notification */
      if (msg_widget == NULL)
      {
         const char *title;

         msg_widget = (menu_widget_msg_t*) calloc(1, sizeof(*msg_widget));

         title = task != NULL ? task->title : msg;

         msg_widget->duration                   = duration;
         msg_widget->offset_y                   = 0;
         msg_widget->alpha                      = 1.0f;

         msg_widget->dying                      = false;
         msg_widget->expired                    = false;

         msg_widget->expiration_timer           = 0;
         msg_widget->task_ptr                   = task;
         msg_widget->expiration_timer_started   = false;

         msg_widget->text_height                = 0;

         msg_widget->unfolded                   = false;
         msg_widget->unfolding                  = false;
         msg_widget->unfold                     = 0.0f;

         if (task)
         {
            msg_widget->msg                  = strdup(title);
            msg_widget->msg_len              = strlen(title);

            msg_widget->task_error           = task->error;
            msg_widget->task_cancelled       = task->cancelled;
            msg_widget->task_finished        = task->finished;
            msg_widget->task_progress        = task->progress;
            msg_widget->task_ident           = task->ident;
            msg_widget->task_title_ptr       = task->title;

            msg_widget->unfolded             = true;

            msg_widget->width                = font_driver_get_message_width(font_regular, title, msg_widget->msg_len, msg_queue_text_scale_factor) + simple_widget_padding/2;

            task->frontend_userdata          = msg_widget;

            msg_widget->hourglass_rotation   = 0;
         }
         else
         {
            /* Compute rect width, wrap if necessary */
            /* Single line text > two lines text > two lines text with expanded width */
            unsigned title_length   = strlen(title);
            char *msg               = strdup(title);
            unsigned width          = msg_queue_default_rect_width;
            unsigned text_width     = font_driver_get_message_width(font_regular, title, title_length, msg_queue_text_scale_factor);
            settings_t *settings    = config_get_ptr();

            msg_widget->text_height = msg_queue_text_scale_factor * settings->floats.video_font_size;

            /* Text is too wide, split it into two lines */
            if (text_width > width)
            {
               if (text_width/2 > width)
               {
                  width = text_width/2;
                  width += 10 * msg_queue_glyph_width;
               }

               word_wrap(msg, msg, title_length/2 + 10, false);

               msg_widget->text_height *= 2.5f;
            }
            else
            {
               width                      = text_width;
               msg_widget->text_height    *= 1.35f;
            }

            msg_widget->msg         = msg;
            msg_widget->msg_len     = strlen(msg);
            msg_widget->width       = width + simple_widget_padding/2;
         }

         fifo_write(msg_queue, &msg_widget, sizeof(msg_widget));
      }
      /* Update task info */
      else
      {
         if (task->title != msg_widget->task_title_ptr)
         {
            unsigned len         = strlen(task->title);
            unsigned new_width   = font_driver_get_message_width(font_regular, task->title, len, msg_queue_text_scale_factor);

            free(msg_widget->msg);

            msg_widget->msg               = strdup(task->title);
            msg_widget->msg_len           = len;
            msg_widget->task_title_ptr    = task->title;

            if (new_width > msg_widget->width)
               msg_widget->width = new_width;
         }

         msg_widget->task_error        = task->error;
         msg_widget->task_cancelled    = task->cancelled;
         msg_widget->task_finished     = task->finished;
         msg_widget->task_progress     = task->progress;
      }
   }

   #ifdef HAVE_THREADS
   runloop_msg_queue_unlock();
   #endif

   return true;
}

bool menu_widgets_msg_queue_push(const char *msg,
      unsigned duration,
      char *title,
      enum message_queue_icon icon, enum message_queue_category category)
{
   return menu_widgets_msg_queue_push_internal(NULL, msg, duration, title, icon, category);
}

static void menu_widgets_unfold_end(void *userdata)
{
   menu_widget_msg_t *unfold = (menu_widget_msg_t*) userdata;

   unfold->unfolding = false;
   moving            = false;
}

static void menu_widgets_move_end(void *userdata)
{
   if (userdata)
   {
      menu_widget_msg_t *unfold = (menu_widget_msg_t*) userdata;
      
      menu_animation_ctx_entry_t entry;

      entry.cb             = menu_widgets_unfold_end;
      entry.duration       = MSG_QUEUE_ANIMATION_DURATION;
      entry.easing_enum    = EASING_OUT_QUAD;
      entry.subject        = &unfold->unfold;
      entry.tag            = generic_tag;
      entry.target_value   = 1.0f;
      entry.userdata       = unfold;

      menu_animation_push(&entry);

      unfold->unfolded  = true;
      unfold->unfolding = true;
   }
   else
   {
      moving = false;
   }
}

static void menu_widgets_msg_queue_expired(void *userdata)
{
   menu_widget_msg_t *msg = (menu_widget_msg_t *) userdata;
   
   if (msg && !msg->expired)
      msg->expired = true;
}

static void menu_widgets_msg_queue_move()
{
   int i;

   menu_widget_msg_t *last    = file_list_get_userdata_at_offset(current_msgs, current_msgs->size-1);
   menu_widget_msg_t *unfold  = NULL; /* there should always be one and only one unfolded message */

   if (current_msgs->size == 0)
      return;

   for (i = current_msgs->size-1; i >= 0; i--)
   {
      menu_widget_msg_t *msg;
      float y;

      msg = file_list_get_userdata_at_offset(current_msgs, i);

      if (!msg || msg->dying)
         continue;

      y += msg_queue_height / (msg->task_ptr ? 2 : 1) + msg_queue_spacing;

      if (!msg->unfolded)
         unfold = msg;

      if (msg->offset_y != y)
      {
         menu_animation_ctx_entry_t entry;

         entry.cb             = i == 0 ? menu_widgets_move_end : NULL;
         entry.duration       = MSG_QUEUE_ANIMATION_DURATION;
         entry.easing_enum    = EASING_OUT_QUAD;
         entry.subject        = &msg->offset_y;
         entry.tag            = generic_tag;
         entry.target_value   = y;
         entry.userdata       = unfold; 

         menu_animation_push(&entry);

         moving = true;
      }
   }
}

static void menu_widgets_msg_queue_free(menu_widget_msg_t *msg, bool touch_list)
{
   int i;
   menu_animation_ctx_tag tag = (uintptr_t) msg;

   /* Kill all animations */
   menu_timer_kill(&msg->hourglass_timer);
   menu_animation_kill_by_tag(&tag);

   /* Free it */
   if (msg->msg)
      free(msg->msg);

   file_list_free_userdata(current_msgs, msg_queue_kill);

   /* Remove it from the list */
   if (touch_list)
   {
      for (i = msg_queue_kill; i < current_msgs->size-1; i++)
      {
         current_msgs->list[i] = current_msgs->list[i+1];
      }

      current_msgs->size--;
   }

   moving = false;
}

static void menu_widgets_msg_queue_kill_end(void *userdata)
{
   menu_widget_msg_t *msg = file_list_get_userdata_at_offset(current_msgs, msg_queue_kill);

   if (!msg)
      return;

   menu_widgets_msg_queue_free(msg, true);
}

static void menu_widgets_msg_queue_kill(unsigned idx)
{
   menu_animation_ctx_entry_t entry;

   menu_widget_msg_t *msg = file_list_get_userdata_at_offset(current_msgs, idx);

   if (!msg)
      return;

   moving = true;
   msg->dying = true;

   msg_queue_kill = idx;

   /* Drop down */
   entry.duration       = MSG_QUEUE_ANIMATION_DURATION;
   entry.easing_enum    = EASING_OUT_QUAD;
   entry.tag            = generic_tag;
   entry.userdata       = NULL;
   entry.subject        = &msg->offset_y;
   entry.target_value   = msg->offset_y - msg_queue_height/4;

   menu_animation_push(&entry);

   /* Fade out */
   entry.cb             = menu_widgets_msg_queue_kill_end;
   entry.subject        = &msg->alpha;
   entry.target_value   = 0.0f;

   menu_animation_push(&entry);

   /* Move all messages back to their correct position */
   menu_widgets_msg_queue_move();
}

static void color_alpha(float *color, float alpha)
{
   color[3] = color[7] = color[11] = color[15] = alpha;
}

static void menu_widgets_draw_icon(
      video_frame_info_t *video_info,
      unsigned icon_width,
      unsigned icon_height,
      uintptr_t texture,
      float x, float y,
      unsigned width, unsigned height,
      float rotation, float scale_factor,
      float *color)
{
   menu_display_ctx_rotate_draw_t rotate_draw;
   menu_display_ctx_draw_t draw;
   struct video_coords coords;
   math_matrix_4x4 mymat;

   rotate_draw.matrix       = &mymat;
   rotate_draw.rotation     = rotation;
   rotate_draw.scale_x      = scale_factor;
   rotate_draw.scale_y      = scale_factor;
   rotate_draw.scale_z      = 1;
   rotate_draw.scale_enable = true;

   menu_display_rotate_z(&rotate_draw, video_info);

   coords.vertices      = 4;
   coords.vertex        = NULL;
   coords.tex_coord     = NULL;
   coords.lut_tex_coord = NULL;
   coords.color         = color;

   draw.x               = x;
   draw.y               = height - y - icon_height;
   draw.width           = icon_width;
   draw.height          = icon_height;
   draw.scale_factor    = scale_factor;
   draw.rotation        = rotation;
   draw.coords          = &coords;
   draw.matrix_data     = &mymat;
   draw.texture         = texture;
   draw.prim_type       = MENU_DISPLAY_PRIM_TRIANGLESTRIP;
   draw.pipeline.id     = 0;

   menu_display_draw(&draw, video_info);
}

static float menu_widgets_get_thumbnail_scale_factor(const float dst_width, const float dst_height,
      const float image_width, const float image_height)
{
   float dst_ratio      = dst_width / dst_height;
   float image_ratio    = image_width / image_height;

   return (dst_ratio > image_ratio) ? dst_height / image_height : dst_width / image_width;
}

static void menu_widgets_screenshot_dispose(void *userdata)
{
   screenshot_loaded = false;
   video_driver_texture_unload(&screenshot_texture);
}

static void menu_widgets_screenshot_end(void *userdata)
{
   menu_animation_ctx_entry_t entry;

   entry.cb             = menu_widgets_screenshot_dispose;
   entry.duration       = MSG_QUEUE_ANIMATION_DURATION;
   entry.easing_enum    = EASING_OUT_QUAD;
   entry.subject        = &screenshot_y;
   entry.tag            = generic_tag;
   entry.target_value   = -((float)screenshot_height);
   entry.userdata       = NULL;

   menu_animation_push(&entry);
}

static void menu_widgets_start_msg_expiration_timer(menu_widget_msg_t *msg_widget, unsigned duration)
{
   if (msg_widget->expiration_timer_started)
      return;

   menu_timer_ctx_entry_t timer;

   timer.cb       = menu_widgets_msg_queue_expired;
   timer.duration = duration;
   timer.userdata = msg_widget;

   menu_timer_start(&msg_widget->expiration_timer, &timer);

   msg_widget->expiration_timer_started = true;
}

static void menu_widgets_hourglass_tick(void *userdata);

/* TODO Kill everything hourglass related when the msg widget is freed */

static void menu_widgets_hourglass_end(void *userdata)
{
   menu_widget_msg_t *msg = (menu_widget_msg_t*) userdata;

   msg->hourglass_rotation = 0.0f;

   menu_timer_ctx_entry_t timer;
   timer.cb       = menu_widgets_hourglass_tick;
   timer.duration = HOURGLASS_INTERVAL;
   timer.userdata = msg;

   menu_timer_start(&msg->hourglass_timer, &timer);
}

static void menu_widgets_hourglass_tick(void *userdata)
{
   menu_widget_msg_t *msg = (menu_widget_msg_t*) userdata;
   menu_animation_ctx_tag tag = (uintptr_t) msg;

   menu_animation_ctx_entry_t entry;

   entry.easing_enum = EASING_OUT_QUAD;
   entry.tag = tag;
   entry.duration = HOURGLASS_DURATION;
   entry.target_value = -(2 * PI);
   entry.subject = &msg->hourglass_rotation;
   entry.cb = menu_widgets_hourglass_end;
   entry.userdata = msg;

   menu_animation_push(&entry);
}

void menu_widgets_iterate()
{
   int i;
   settings_t *settings = config_get_ptr();

   if (!init)
      return;

   /* Messages queue */
   #ifdef HAVE_THREADS
   runloop_msg_queue_lock();
   #endif

   /* Consume one message if available */
   if (fifo_read_avail(msg_queue) > 0 && !moving && current_msgs->size < MSG_QUEUE_ONSCREEN_MAX)
   {
      menu_widget_msg_t *msg_widget;

      fifo_read(msg_queue, &msg_widget, sizeof(msg_widget));

      if (file_list_append(current_msgs,
            NULL, 
            NULL, 
            0, 
            0, 
            0))
      {
         file_list_set_userdata(current_msgs, current_msgs->size-1, msg_widget);

         /* Start expiration timer if not associated to a task */
         if (msg_widget->task_ptr == NULL)
         {
            menu_widgets_start_msg_expiration_timer(msg_widget, MSG_QUEUE_ANIMATION_DURATION + msg_widget->duration);
         }
         /* Else, start hourglass animation timer */
         else
         {
            menu_widgets_hourglass_end(msg_widget);
         }

         menu_widgets_msg_queue_move();
      }
   }

   #ifdef HAVE_THREADS
   runloop_msg_queue_unlock();
   #endif

   /* Kill first expired message */
   /* Start expiration timer of dead tasks */
   for (i = 0; i < current_msgs->size ; i++)
   {
      menu_widget_msg_t *msg  = file_list_get_userdata_at_offset(current_msgs, i);

      if (!msg)
         continue;

      if (msg->task_ptr != NULL && (msg->task_finished || msg->task_cancelled))
         menu_widgets_start_msg_expiration_timer(msg, TASK_FINISHED_DURATION);

      if (msg->expired && !moving)
      {
         menu_widgets_msg_queue_kill(i);
         break;
      }
   }

   /* Load screenshot and start its animation */
   if (screenshot_filename[0] != '\0')
   {
      menu_animation_ctx_entry_t entry;
      menu_timer_ctx_entry_t timer;
      unsigned width;

      video_driver_texture_unload(&screenshot_texture);
      menu_display_reset_textures_list(screenshot_filename, "", &screenshot_texture, TEXTURE_FILTER_MIPMAP_LINEAR, &screenshot_texture_width, &screenshot_texture_height);

      settings_t *settings = config_get_ptr();

      video_driver_get_size(&width, NULL);

      screenshot_height = settings->floats.video_font_size * 4;
      screenshot_width  = width;

      screenshot_scale_factor = menu_widgets_get_thumbnail_scale_factor(
         width, screenshot_height,
         screenshot_texture_width, screenshot_texture_height
      );

      screenshot_thumbnail_width  = screenshot_texture_width * screenshot_scale_factor;
      screenshot_thumbnail_height = screenshot_texture_height * screenshot_scale_factor;

      screenshot_shotname_length  = (width - screenshot_thumbnail_width - simple_widget_padding*2) / glyph_width;

      screenshot_y = -((float)screenshot_height);

      entry.cb             = NULL;
      entry.duration       = MSG_QUEUE_ANIMATION_DURATION;
      entry.easing_enum    = EASING_OUT_QUAD;
      entry.subject        = &screenshot_y;
      entry.tag            = generic_tag;
      entry.target_value   = 0.0f;
      entry.userdata       = NULL;

      menu_animation_push(&entry);

      timer.cb       = menu_widgets_screenshot_end;
      timer.duration = SCREENSHOT_NOTIFICATION_DURATION;
      timer.userdata = NULL;

      menu_timer_start(&screenshot_timer, &timer);

      screenshot_loaded       = true;
      screenshot_filename[0]  = '\0';
   }
}

static int menu_widgets_draw_indicator(video_frame_info_t *video_info, 
      menu_texture_item icon, int y, int top_right_x_advance, 
      enum msg_hash_enums msg)
{
   unsigned width;
   settings_t *settings = config_get_ptr();

   color_alpha(backdrop, DEFAULT_BACKDROP);

   if (icon)
   {
      unsigned height = simple_widget_height * 2;
      width  = height;

      menu_display_draw_quad(video_info, 
         top_right_x_advance - width, y,
         width, height, 
         video_info->width, video_info->height,
         backdrop
      );

      color_alpha(menu_widgets_pure_white, 1.0f);

      menu_display_blend_begin(video_info);
      menu_widgets_draw_icon(video_info, width, height,
         icon, top_right_x_advance - width, y,
         video_info->width, video_info->height,
         0, 1, menu_widgets_pure_white
      );
      menu_display_blend_end(video_info);
   }
   else
   {
      unsigned height = simple_widget_height;
      const char *txt = msg_hash_to_str(msg);
      unsigned width = font_driver_get_message_width(font_regular, txt, strlen(txt), 1) + simple_widget_padding*2;

      menu_display_draw_quad(video_info, 
         top_right_x_advance - width, y,
         width, height, 
         video_info->width, video_info->height,
         backdrop
      );

      menu_display_draw_text(font_regular,
         txt, 
         top_right_x_advance - width + simple_widget_padding, settings->floats.video_font_size + simple_widget_padding/4,
         video_info->width, video_info->height,
         0xFFFFFFFF, TEXT_ALIGN_LEFT,
         1.0f,
         false, 0, false
      );
   }

   return width;
}

static void menu_widgets_draw_task_msg(menu_widget_msg_t *msg, video_frame_info_t *video_info)
{
   /* TODO Text animation and progress bar color change */

   unsigned text_color;
   unsigned rect_width;
   unsigned bar_width;

   float *msg_queue_current_background;

   unsigned task_percentage_offset = 0;
   char task_percentage[256]       = {0};
   settings_t *settings            = config_get_ptr();

   task_percentage_offset = glyph_width * (msg->task_error ? 12 : 5) + simple_widget_padding * 1.25f; /*11 = strlen("Task failed")+1 */

   if (msg->task_finished)
   {
      if (msg->task_error)
      {
         snprintf(task_percentage, sizeof(task_percentage), "Task failed");
      }
      else
      {
         snprintf(task_percentage, sizeof(task_percentage), "100%%");
      }
   }
   else if (msg->task_progress >= 0 && msg->task_progress <= 100)
   {
      snprintf(task_percentage, sizeof(task_percentage), "%i%%", msg->task_progress);
   }

   rect_width = simple_widget_padding + msg->width + task_percentage_offset;
   bar_width  = rect_width * msg->task_progress/100.0f;
   text_color = COLOR_TEXT_ALPHA(0xFFFFFF00, (unsigned)(msg->alpha*255.0f));

   /* Rect */
   /* TODO Correctly change rect color depending on stage */
   msg_queue_current_background = msg->task_finished ? msg_queue_task_progress_1 : msg_queue_background;
   color_alpha(msg_queue_current_background, msg->alpha);
   menu_display_draw_quad(video_info,
      msg_queue_rect_start_x - msg_queue_icon_size_x, video_info->height - msg->offset_y,
      rect_width, msg_queue_height/2,
      video_info->width, video_info->height,
      msg_queue_current_background
   );

   /* Progress bar */
   /* TODO Change progress bar color depending on stage */
   color_alpha(msg_queue_task_progress_1, 1.0f);
   if (!msg->task_finished && msg->task_progress >= 0 && msg->task_progress <= 100)
   {
      menu_display_draw_quad(video_info,
         msg_queue_rect_start_x - msg_queue_icon_size_x, video_info->height - msg->offset_y,
         bar_width, msg_queue_height/2,
         video_info->width, video_info->height,
         msg_queue_task_progress_1
      );
   }

   /* Icon */
   color_alpha(menu_widgets_pure_white, msg->alpha);
   menu_display_blend_begin(video_info);
   menu_widgets_draw_icon(video_info,
      msg_queue_height/2,
      msg_queue_height/2,
      hourglass_icon,
      msg_queue_rect_start_x - msg_queue_icon_size_x,
      video_info->height - msg->offset_y,
      video_info->width,
      video_info->height, msg->hourglass_rotation, 1, menu_widgets_pure_white);
   menu_display_blend_end(video_info);

   /* Text */
   if ((text_color & 0x000000FF) != 0)
   {
      menu_display_draw_text(font_regular,
         msg->msg,
         msg_queue_rect_start_x - msg_queue_icon_size_x + msg_queue_height/2,
         video_info->height - msg->offset_y + msg_queue_text_scale_factor * settings->floats.video_font_size + msg_queue_height/4 - settings->floats.video_font_size/2.25f,
         video_info->width, video_info->height,
         text_color,
         TEXT_ALIGN_LEFT,
         msg_queue_text_scale_factor,
         false,
         0,
         true
      );
   }

   /* Progress text */
   text_color = COLOR_TEXT_ALPHA(0x9E9D9E00, (unsigned)(msg->alpha*255.0f));
   if ((text_color & 0x000000FF) != 0)
   {
      menu_display_draw_text(font_regular,
         task_percentage,
         msg_queue_rect_start_x - msg_queue_icon_size_x + rect_width - msg_queue_glyph_width,
         video_info->height - msg->offset_y + msg_queue_text_scale_factor * settings->floats.video_font_size + msg_queue_height/4 - settings->floats.video_font_size/2.25f,
         video_info->width, video_info->height,
         text_color,
         TEXT_ALIGN_RIGHT,
         msg_queue_text_scale_factor,
         false,
         0,
         true
      );
   }
}

static void menu_widgets_draw_regular_msg(menu_widget_msg_t *msg, video_frame_info_t *video_info)
{
   menu_texture_item icon     = 0;
   settings_t *settings       = config_get_ptr();

   unsigned bar_width;
   unsigned text_color;

   if (!icon)
      icon = info_icon; /* TODO Real icon logic here */

   /* Icon */
   color_alpha(msg_queue_info, msg->alpha);
   color_alpha(menu_widgets_pure_white, msg->alpha);
   color_alpha(msg_queue_background, msg->alpha);

   menu_display_blend_begin(video_info);

   if (!msg->unfolded || msg->unfolding)
      menu_display_scissor_begin(video_info, msg_queue_scissor_start_x, 0,
         (msg_queue_scissor_start_x + msg->width - simple_widget_padding*2) * msg->unfold, video_info->height);

   /* (int) cast is to be consistent with the rect drawing and prevent alignment
      * issues, don't remove it */
   menu_widgets_draw_icon(video_info,
      msg_queue_icon_size_x, msg_queue_icon_size_y,
      msg_queue_icon_rect, msg_queue_spacing, (int)(video_info->height - msg->offset_y - msg_queue_icon_offset_y),
      video_info->width, video_info->height, 
      0, 1, msg_queue_background);

   menu_display_blend_end(video_info);

   /* Background */
   bar_width = simple_widget_padding + msg->width;

   menu_display_draw_quad(video_info,
      msg_queue_rect_start_x, video_info->height - msg->offset_y,
      bar_width, msg_queue_height,
      video_info->width, video_info->height,
      msg_queue_background
   );

   /* Text */
   text_color = COLOR_TEXT_ALPHA(0xFFFFFF00, (unsigned)(msg->alpha*255.0f));

   if ((text_color & 0x000000FF) != 0)
   {
      menu_display_draw_text(font_regular,
         msg->msg,
         msg_queue_rect_start_x + simple_widget_padding/2 - ((1.0f-msg->unfold) * (msg->width + simple_widget_padding)/2),
         video_info->height - msg->offset_y + settings->floats.video_font_size * msg_queue_text_scale_factor + msg_queue_height/2 - msg->text_height/2,
         video_info->width, video_info->height,
         text_color,
         TEXT_ALIGN_LEFT,
         msg_queue_text_scale_factor, false, 0, true
      );
   }

   if (!msg->unfolded || msg->unfolding)
   {
      font_driver_flush(video_info->width, video_info->height, font_regular, video_info);
      font_driver_flush(video_info->width, video_info->height, font_bold, video_info);

      font_raster_regular.carr.coords.vertices = 0;
      font_raster_bold.carr.coords.vertices = 0;

      menu_display_scissor_end(video_info);
   }

   menu_display_blend_begin(video_info);

   menu_widgets_draw_icon(video_info,
      msg_queue_icon_size_x, msg_queue_icon_size_y,
      msg_queue_icon, msg_queue_spacing, video_info->height - msg->offset_y - msg_queue_icon_offset_y, 
      video_info->width, video_info->height,
      0, 1, msg_queue_info);

   menu_widgets_draw_icon(video_info,
      msg_queue_icon_size_x, msg_queue_icon_size_y,
      msg_queue_icon_outline, msg_queue_spacing, video_info->height - msg->offset_y - msg_queue_icon_offset_y, 
      video_info->width, video_info->height,
      0, 1, menu_widgets_pure_white);

   menu_widgets_draw_icon(video_info,
      msg_queue_internal_icon_size, msg_queue_internal_icon_size,
      icon, msg_queue_spacing + msg_queue_internal_icon_offset, video_info->height - msg->offset_y - msg_queue_icon_offset_y + msg_queue_internal_icon_offset, 
      video_info->width, video_info->height,
      0, 1, menu_widgets_pure_white);
   
   menu_display_blend_end(video_info);
}

void menu_widgets_frame(video_frame_info_t *video_info)
{
   int i;
   int top_right_x_advance = video_info->width;

   settings_t *settings = config_get_ptr();

   if (!init)
      return;

   menu_widgets_frame_count++;

   menu_display_set_viewport(video_info->width, video_info->height);

   /* Font setup */
   font_driver_bind_block(font_regular, &font_raster_regular);
   font_driver_bind_block(font_bold, &font_raster_bold);

   font_raster_regular.carr.coords.vertices = 0;
   font_raster_bold.carr.coords.vertices = 0;

   /* Screenshot */
   /* TODO Make this a function and use it for cheevos (width as a parameter) */
   if (screenshot_loaded)
   {
      char shotname[256];
      menu_animation_ctx_ticker_t ticker;

      color_alpha(backdrop, DEFAULT_BACKDROP);

      menu_display_draw_quad(video_info,
         0, screenshot_y, 
         screenshot_width, screenshot_height,
         video_info->width, video_info->height,
         backdrop
      );

      color_alpha(menu_widgets_pure_white, 1.0f);
      menu_widgets_draw_icon(video_info, 
         screenshot_thumbnail_width, screenshot_thumbnail_height, 
         screenshot_texture, 
         0, screenshot_y, 
         video_info->width, video_info->height, 
         0, 1, menu_widgets_pure_white
      );
   
      menu_display_draw_text(font_regular,
         msg_hash_to_str(MSG_SCREENSHOT_SAVED),
         screenshot_thumbnail_width + simple_widget_padding, settings->floats.video_font_size * 1.9f + screenshot_y,
         video_info->width, video_info->height,
         text_color_faint,
         TEXT_ALIGN_LEFT,
         1, false, 0, true
      );

      ticker.idx        = menu_animation_get_ticker_time();
      ticker.len        = screenshot_shotname_length;
      ticker.s          = shotname;
      ticker.selected   = true;
      ticker.str        = screenshot_shotname;

      menu_animation_ticker(&ticker);

      menu_display_draw_text(font_regular,
         shotname,
         screenshot_thumbnail_width + simple_widget_padding, settings->floats.video_font_size * 2.9f + screenshot_y,
         video_info->width, video_info->height,
         text_color_info,
         TEXT_ALIGN_LEFT,
         1, false, 0, true
      );
   }

   /* Volume */
   if (volume_alpha > 0.0f)
   {
      char msg[255];
      char percentage_msg[255];

      menu_texture_item volume_icon = 0;

      unsigned volume_width         = video_info->width / 3;
      unsigned volume_height        = settings->floats.video_font_size * 4;
      unsigned icon_size            = volume_icon_med ? volume_height : simple_widget_padding;
      unsigned text_color           = COLOR_TEXT_ALPHA(0xffffffff, (unsigned)(volume_text_alpha*255.0f));
      unsigned text_color_db        = COLOR_TEXT_ALPHA(text_color_faint, (unsigned)(volume_text_alpha*255.0f));

      unsigned bar_x          = icon_size;
      unsigned bar_height     = settings->floats.video_font_size/2;
      unsigned bar_width      = volume_width - bar_x - simple_widget_padding;
      unsigned bar_y          = volume_height / 2 + bar_height/2;

      float *bar_background = NULL;
      float *bar_foreground = NULL;
      float bar_percentage  = 0.0f;

      if (volume_mute)
      {
         volume_icon = volume_icon_mute;
      }
      else if (volume_percent <= 1.0f)
      {
         if (volume_percent <= 0.5f)
            volume_icon = volume_icon_min;
         else
            volume_icon = volume_icon_med;

         bar_background = volume_bar_background;
         bar_foreground = volume_bar_normal;
         bar_percentage = volume_percent;
      }
      else if (volume_percent > 1.0f && volume_percent <= 2.0f)
      {
         volume_icon = volume_icon_max;

         bar_background = volume_bar_normal;
         bar_foreground = volume_bar_loud;
         bar_percentage = volume_percent - 1.0f;
      }
      else
      {
         volume_icon = volume_icon_max;

         bar_background = volume_bar_loud;
         bar_foreground = volume_bar_loudest;
         bar_percentage = volume_percent - 2.0f;
      }

      if (bar_percentage > 1.0f)
         bar_percentage = 1.0f;

      /* Backdrop */
      color_alpha(backdrop, volume_alpha);

      menu_display_draw_quad(video_info,
         0, 0,
         volume_width,
         volume_height,
         video_info->width,
         video_info->height,
         backdrop
      );

      /* Icon */
      if (volume_icon)
      {
         color_alpha(menu_widgets_pure_white, volume_text_alpha);

         menu_display_blend_begin(video_info);
         menu_widgets_draw_icon(video_info,
            icon_size, icon_size,
            volume_icon,
            0, 0, 
            video_info->width, video_info->height,
            0, 1, menu_widgets_pure_white
         );
         menu_display_blend_end(video_info);
      }

      if (volume_mute)
      {
         if (!volume_icon_mute)
         {
            const char *text  = msg_hash_to_str(MSG_AUDIO_MUTED);
            if ((text_color & 0x000000FF) != 0)
            {
               menu_display_draw_text(font_regular,
                  text,
                  volume_width/2, volume_height/2 + settings->floats.video_font_size/3,
                  video_info->width, video_info->height,
                  text_color, TEXT_ALIGN_CENTER,
                  1, false, 0, false
               );
            }
         }
      }
      else
      {
         /* Bar */
         color_alpha(bar_background, volume_text_alpha);
         color_alpha(bar_foreground, volume_text_alpha);

         menu_display_draw_quad(video_info,
            bar_x + bar_percentage * bar_width, bar_y,
            bar_width - bar_percentage * bar_width, bar_height,
            video_info->width, video_info->height,
            bar_background
         );

         menu_display_draw_quad(video_info,
            bar_x, bar_y,
            bar_percentage * bar_width, bar_height,
            video_info->width, video_info->height,
            bar_foreground
         );

         /* Text */
         snprintf(msg, sizeof(msg), (volume_db >= 0 ? "+%.1f dB" : "%.1f dB"),
            volume_db);

         snprintf(percentage_msg, sizeof(percentage_msg), "%d%%",
            (int)(volume_percent * 100.0f));

         menu_display_draw_text(font_regular,
            msg, 
            volume_width - simple_widget_padding, settings->floats.video_font_size * 2,
            video_info->width, video_info->height,
            text_color_db,
            TEXT_ALIGN_RIGHT,
            1, false, 0, false
         );

         menu_display_draw_text(font_regular,
            percentage_msg,
            icon_size, settings->floats.video_font_size * 2,
            video_info->width, video_info->height,
            text_color,
            TEXT_ALIGN_LEFT,
            1, false, 0, false
         );
      }
   }

   /* Draw all messages */
   for (i = current_msgs->size-1; i >= 0 ; i--)
   {
      menu_widget_msg_t *msg = file_list_get_userdata_at_offset(current_msgs, i);

      if (!msg)
         continue;

      if (msg->task_ptr)
         menu_widgets_draw_task_msg(msg, video_info);
      else
         menu_widgets_draw_regular_msg(msg, video_info);
   }

   /* FPS Counter */
   if (video_info->fps_show || video_info->framecount_show)
   {
      char *text = *fps_text == '\0' ? "n/a" : fps_text;

      int text_width = font_driver_get_message_width(font_regular, text, strlen(text), 1.0f);
      int total_width = text_width + simple_widget_padding * 2;

      color_alpha(backdrop, DEFAULT_BACKDROP);

      menu_display_draw_quad(video_info,
         top_right_x_advance - total_width, 0,
         total_width, simple_widget_height,
         video_info->width, video_info->height,
         backdrop
      );

      menu_display_draw_text(font_regular,
         text,
         top_right_x_advance - simple_widget_padding - text_width, settings->floats.video_font_size + simple_widget_padding/4,
         video_info->width, video_info->height,
         0xFFFFFFFF,
         TEXT_ALIGN_LEFT,
         1, false,0, true
      );
   }

   /* Indicators */
   if (paused)
      top_right_x_advance -= menu_widgets_draw_indicator(video_info,
         paused_icon, (video_info->fps_show ? simple_widget_height : 0), top_right_x_advance,
         MSG_PAUSED);

   if (fast_forward)
      top_right_x_advance -= menu_widgets_draw_indicator(video_info,
         fast_forward_icon, (video_info->fps_show ? simple_widget_height : 0), top_right_x_advance,
         MSG_PAUSED);

   if (rewinding)
      top_right_x_advance -= menu_widgets_draw_indicator(video_info,
         rewind_icon, (video_info->fps_show ? simple_widget_height : 0), top_right_x_advance,
         MSG_REWINDING);

   if (video_info->runloop_is_slowmotion)
      top_right_x_advance -= menu_widgets_draw_indicator(video_info,
         slowmotion_icon, (video_info->fps_show ? simple_widget_height : 0), top_right_x_advance,
         MSG_SLOW_MOTION);

   /* Screenshot */
   if (screenshot_alpha > 0.0f)
   {
      color_alpha(menu_widgets_pure_white, screenshot_alpha);
      menu_display_draw_quad(video_info,
         0, 0,
         video_info->width, video_info->height,
         video_info->width, video_info->height,
         menu_widgets_pure_white
      );
   }

   font_driver_flush(video_info->width, video_info->height, font_regular, video_info);
   font_driver_flush(video_info->width, video_info->height, font_bold, video_info);

   menu_display_unset_viewport(video_info->width, video_info->height);
}

void menu_widgets_init(bool video_is_threaded)
{
   settings_t *settings = config_get_ptr();

   init = true;

   if (!menu_display_init_first_driver(video_is_threaded))
      goto err;

   menu_widgets_frame_count = 0;

   fps_text[0] = '\0';

   msg_queue = fifo_new(MSG_QUEUE_PENDING_MAX * sizeof(menu_widget_msg_t*));

   if (!msg_queue)
      goto err;

   current_msgs = (file_list_t*)calloc(1, sizeof(file_list_t));

   if (!current_msgs)
      goto err;

   file_list_reserve(current_msgs, MSG_QUEUE_ONSCREEN_MAX);

   return;
err:
   menu_widgets_free();
}

void menu_widgets_context_reset(bool is_threaded)
{
   char xmb_path[PATH_MAX_LENGTH];
   char menu_widgets_path[PATH_MAX_LENGTH];
   char theme_path[PATH_MAX_LENGTH];

   char monochrome_png_path[PATH_MAX_LENGTH];

   char ozone_path[PATH_MAX_LENGTH];
   char font_path[PATH_MAX_LENGTH];

   settings_t *settings = config_get_ptr();

   unsigned video_info_width;

   video_driver_get_size(&video_info_width, NULL);

   /* Textures paths */
   fill_pathname_join(
      menu_widgets_path,
      settings->paths.directory_assets,
      "menu_widgets",
      sizeof(menu_widgets_path)
   );

   fill_pathname_join(
      xmb_path,
      settings->paths.directory_assets,
      "xmb",
      sizeof(xmb_path)
   );

   /* Monochrome */
   fill_pathname_join(
      theme_path,
      xmb_path,
      "monochrome",
      sizeof(theme_path)
   );

   fill_pathname_join(
      monochrome_png_path,
      theme_path,
      "png",
      sizeof(monochrome_png_path)
   );

   /* Load textures */
   /* Monochrome */
   menu_display_reset_textures_list("menu_volume_med.png", monochrome_png_path, &volume_icon_med, TEXTURE_FILTER_MIPMAP_LINEAR, NULL, NULL);
   menu_display_reset_textures_list("menu_volume_max.png", monochrome_png_path, &volume_icon_max, TEXTURE_FILTER_MIPMAP_LINEAR, NULL, NULL);
   menu_display_reset_textures_list("menu_volume_min.png", monochrome_png_path, &volume_icon_min, TEXTURE_FILTER_MIPMAP_LINEAR, NULL, NULL);
   menu_display_reset_textures_list("menu_volume_mute.png", monochrome_png_path, &volume_icon_mute, TEXTURE_FILTER_MIPMAP_LINEAR, NULL, NULL);
   menu_display_reset_textures_list("menu_pause.png", monochrome_png_path, &paused_icon, TEXTURE_FILTER_MIPMAP_LINEAR, NULL, NULL);
   menu_display_reset_textures_list("menu_frameskip.png", monochrome_png_path, &fast_forward_icon, TEXTURE_FILTER_MIPMAP_LINEAR, NULL, NULL);
   menu_display_reset_textures_list("menu_rewind.png", monochrome_png_path, &rewind_icon, TEXTURE_FILTER_MIPMAP_LINEAR, NULL, NULL);
   menu_display_reset_textures_list("resume.png", monochrome_png_path, &slowmotion_icon, TEXTURE_FILTER_MIPMAP_LINEAR, NULL, NULL);
   menu_display_reset_textures_list("menu_hourglass.png", monochrome_png_path, &hourglass_icon, TEXTURE_FILTER_MIPMAP_LINEAR, NULL, NULL);
   menu_display_reset_textures_list("menu_info.png", monochrome_png_path, &info_icon, TEXTURE_FILTER_MIPMAP_LINEAR, NULL, NULL);

   /* Menu widgets */
   menu_display_reset_textures_list("msg_queue_icon.png", menu_widgets_path, &msg_queue_icon, TEXTURE_FILTER_LINEAR, NULL, NULL);
   menu_display_reset_textures_list("msg_queue_icon_outline.png", menu_widgets_path, &msg_queue_icon_outline, TEXTURE_FILTER_LINEAR, NULL, NULL);
   menu_display_reset_textures_list("msg_queue_icon_rect.png", menu_widgets_path, &msg_queue_icon_rect, TEXTURE_FILTER_NEAREST, NULL, NULL);

   /* Fonts paths */
      fill_pathname_join(
      ozone_path,
      settings->paths.directory_assets,
      "ozone",
      sizeof(ozone_path)
   );

   /* Fonts */
   fill_pathname_join(font_path, ozone_path, "regular.ttf", sizeof(font_path));
   font_regular = menu_display_font_file(font_path, settings->floats.video_font_size, is_threaded);

   fill_pathname_join(font_path, ozone_path, "bold.ttf", sizeof(font_path));
   font_bold = menu_display_font_file(font_path, settings->floats.video_font_size, is_threaded);

   /* Metrics */
   simple_widget_padding = settings->floats.video_font_size * 2/3;
   simple_widget_height = settings->floats.video_font_size + simple_widget_padding;
   glyph_width = font_driver_get_message_width(font_regular, "a", 1, 1);

   msg_queue_height                 = settings->floats.video_font_size * 2.5f;
   msg_queue_icon_size_y            = msg_queue_height * 1.2347826087f; /* original image is 280x284 */
   msg_queue_icon_size_x            = 0.98591549295f * msg_queue_icon_size_y;
   msg_queue_text_scale_factor      = 0.69f;
   msg_queue_base_width             = video_info_width / 4;
   msg_queue_spacing                = msg_queue_height / 3;
   msg_queue_glyph_width            = glyph_width * msg_queue_text_scale_factor;
   msg_queue_rect_start_x           = msg_queue_spacing + msg_queue_icon_size_x;
   msg_queue_internal_icon_size     = msg_queue_icon_size_y;
   msg_queue_internal_icon_offset   = (msg_queue_icon_size_y - msg_queue_internal_icon_size)/2;
   msg_queue_icon_offset_y          = (msg_queue_icon_size_y - msg_queue_height)/2;
   msg_queue_scissor_start_x        = msg_queue_spacing + msg_queue_icon_size_x - (msg_queue_icon_size_x * 0.28928571428);
   msg_queue_default_rect_width     = msg_queue_glyph_width * 40;
}

void menu_widgets_context_destroy()
{
   /* Textures */
   video_driver_texture_unload(&volume_icon_med);
   video_driver_texture_unload(&volume_icon_max);
   video_driver_texture_unload(&volume_icon_min);
   video_driver_texture_unload(&volume_icon_mute);
   video_driver_texture_unload(&paused_icon);
   video_driver_texture_unload(&fast_forward_icon);
   video_driver_texture_unload(&rewind_icon);
   video_driver_texture_unload(&slowmotion_icon);

   video_driver_texture_unload(&hourglass_icon);
   video_driver_texture_unload(&info_icon);
   video_driver_texture_unload(&msg_queue_icon);
   video_driver_texture_unload(&msg_queue_icon_outline);
   video_driver_texture_unload(&msg_queue_icon_rect);

   /* Fonts */
   menu_display_font_free(font_regular);
   menu_display_font_free(font_bold);

   font_regular = NULL;
   font_bold = NULL;
}

void menu_widgets_free()
{
   int i;

   init = false;

   /* Kill any pending animation */
   menu_animation_kill_by_tag(&volume_tag);
   menu_animation_kill_by_tag(&generic_tag);

   /* Purge everything from the fifo */
   if (msg_queue)
   {
      while (fifo_read_avail(msg_queue) > 0)
      {
         menu_widget_msg_t *msg_widget;

         fifo_read(msg_queue, &msg_widget, sizeof(msg_widget));

         menu_widgets_msg_queue_free(msg_widget, false);
      }

      fifo_free(msg_queue);
   }

   /* Purge everything from the list */
   if (current_msgs) 
   {
      for (i = 0; i < current_msgs->size; i++)
      {
         menu_widget_msg_t *msg = file_list_get_userdata_at_offset(current_msgs, i);

         menu_widgets_msg_queue_free(msg, false);
      }
      file_list_free(current_msgs);
   }

   video_driver_texture_unload(&screenshot_texture);

   /* Font */
   video_coord_array_free(&font_raster_regular.carr);
   video_coord_array_free(&font_raster_bold.carr);

   font_driver_bind_block(NULL, NULL);
}

static void menu_widgets_volume_timer_end()
{
   menu_animation_ctx_entry_t entry;

   entry.cb             = NULL;
   entry.duration       = MSG_QUEUE_ANIMATION_DURATION;
   entry.easing_enum    = EASING_OUT_QUAD;
   entry.subject        = &volume_alpha;
   entry.tag            = volume_tag;
   entry.target_value   = 0.0f;
   entry.userdata       = NULL;

   menu_animation_push(&entry);

   entry.subject        = &volume_text_alpha;

   menu_animation_push(&entry);
}

bool menu_widgets_volume_update_and_show()
{
   settings_t *settings = config_get_ptr();
   bool mute            = *(audio_get_bool_ptr(AUDIO_ACTION_MUTE_ENABLE));
   float new_volume     = settings->floats.audio_volume;
   menu_timer_ctx_entry_t entry;

   if (!init)
      return false;

   menu_animation_kill_by_tag(&volume_tag);

   volume_db         = new_volume;
   volume_percent    = pow(10, new_volume/20);
   volume_alpha      = DEFAULT_BACKDROP;
   volume_text_alpha = 1.0f;
   volume_mute       = mute;

   entry.cb       = menu_widgets_volume_timer_end;
   entry.duration = VOLUME_DURATION;
   entry.userdata = NULL;

   menu_timer_start(&volume_timer, &entry);

   return true;
}

bool menu_widgets_set_fps_text(char *new_fps_text)
{
   if (!init)
      return false;

   strlcpy(fps_text, new_fps_text, sizeof(fps_text));

   return true;
}

bool menu_widgets_set_fast_forward(bool is_fast_forward)
{
   if (!init)
      return false;

   fast_forward = is_fast_forward;

   return true;
}

bool menu_widgets_set_rewind(bool is_rewind)
{
   if (!init)
      return false;

   rewinding = is_rewind;

   return true;
}

static void menu_widgets_screenshot_fadeout(void *userdata)
{
   menu_animation_ctx_entry_t entry;

   if (!init)
      return;

   entry.cb             = menu_widgets_screenshot_fadeout;
   entry.duration       = SCREENSHOT_DURATION_OUT;
   entry.easing_enum    = EASING_OUT_QUAD;
   entry.subject        = &screenshot_alpha;
   entry.tag            = generic_tag;
   entry.target_value   = 0.0f;
   entry.userdata       = NULL;

   menu_animation_push(&entry);
}

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void menu_widgets_screenshot_taken(const char *shotname, const char *filename)
{
   strlcpy(screenshot_filename, filename, sizeof(screenshot_filename));
   strlcpy(screenshot_shotname, shotname, sizeof(screenshot_shotname));
}

void menu_widgets_take_screenshot()
{
   menu_animation_ctx_entry_t entry;

   if (!init)
      return;

   entry.cb             = menu_widgets_screenshot_fadeout;
   entry.duration       = SCREENSHOT_DURATION_IN;
   entry.easing_enum    = EASING_IN_QUAD;
   entry.subject        = &screenshot_alpha;
   entry.tag            = generic_tag;
   entry.target_value   = 1.0f;
   entry.userdata       = NULL;

   menu_animation_push(&entry);
}

bool menu_widgets_task_msg_queue_push(retro_task_t *task)
{
   if (!init)
      return false;

   if (task->title != NULL && !task->mute)
      menu_widgets_msg_queue_push_internal(task, NULL, 0, NULL, MESSAGE_QUEUE_CATEGORY_INFO, MESSAGE_QUEUE_ICON_DEFAULT);

   return true;
}