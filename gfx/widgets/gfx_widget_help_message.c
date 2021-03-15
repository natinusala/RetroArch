/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2021 - natinusala
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

#include "../gfx_widgets.h"
#include "../gfx_display.h"

#include <string/stdstring.h>

// TODO: handle threading
// TODO: make sure it's C89 compliant
// TODO: ensure counter works

/* Max title length (before truncating!) */
#define TITLE_MAX_LENGTH 64 // TODO: ensure creating a widget with a longer title truncates it and doesn't corrupt anything

/* Max message length (after word wrapping!) */
#define MESSAGE_MAX_LENGTH 2048 // TODO: ensure creating a widget with a longer message truncates it and doesn't corrupt anything

enum gfx_widget_help_message_slot_state
{
   GFX_WIDGET_HELP_MESSAGE_STATE_NONE = 0, /* default state */
   GFX_WIDGET_HELP_MESSAGE_STATE_SLIDING_IN, /* a message is currently sliding in */
   GFX_WIDGET_HELP_MESSAGE_STATE_IDLE, /* a message is visible and waiting for timeout or change its text */
   GFX_WIDGET_HELP_MESSAGE_STATE_TEXT_CHANGING, /* a message is visible and its text is currently changing */
   GFX_WIDGET_HELP_MESSAGE_STATE_SLIDING_OUT, /* a message is currently sliding out */
};

/* Single help message slot state */
typedef struct gfx_widget_help_message_slot
{
   enum help_message_slot slot;

   enum gfx_widget_help_message_slot_state state;

   /* Goes from 1.0f to 0.0f, represents the slide in / out animation.
      Formula is x = x - (width * slide_animation) */
   float slide_animation;

   gfx_timer_t timeout_timer;

   /* Currently displayed message data */
   char original_title[TITLE_MAX_LENGTH]; /* original title, truncated into truncated_title */
   char original_message[MESSAGE_MAX_LENGTH]; /* original message, word wrapped into wrapped_message */

   char truncated_title[TITLE_MAX_LENGTH]; /* actually displayed title */
   char wrapped_message[MESSAGE_MAX_LENGTH]; /* actually displayed text */

   /* Info about the next queued message, if any */
   struct
   {
      char* title; /* strdup'd */
      char* message; /* strdup'd */

      bool animated; /* for slide in animation */

      retro_time_t timeout;
   } next;

   struct
   {
      int x;
      int y;
      int width;

      int header_height;
      int message_height;
   } layout;
} gfx_widget_help_message_slot_t;

/* Global help message "widget" state */
struct gfx_widget_help_message_state
{
   gfx_widget_help_message_slot_t* slots[_HELP_MESSAGE_SLOT_MAX];

   int active_slots; /* used to know if we should draw anything */

   struct {
      int header_height;
      int glyph_width;
      int line_height;
      int display_width;
      int display_height;
      int pellet_offset_y;
      int title_offset_x;
      int title_padding_x;
      int message_padding_x;
      int padding_y;
      int pellet_size;
   } layout;
};

typedef struct gfx_widget_help_message_state gfx_widget_help_message_state_t;

static gfx_widget_help_message_state_t p_w_help_message_st;

/* Widget impl */

gfx_widget_help_message_slot_t* gfx_widget_help_message_slot_prepare(enum help_message_slot slot)
{
   gfx_widget_help_message_state_t* state = &p_w_help_message_st;

   /* Slots are lazily allocated to avoid taking useless RAM on boot */
   gfx_widget_help_message_slot_t* slot_ptr = state->slots[(size_t) slot];

   if (slot_ptr)
      return slot_ptr;

   slot_ptr = calloc(1, sizeof(*slot_ptr));

   /* Slot init */
   slot_ptr->state = GFX_WIDGET_HELP_MESSAGE_STATE_NONE;

   state->slots[(size_t) slot] = slot_ptr;

   return slot_ptr;
}

void gfx_widget_help_message_slot_release(enum help_message_slot slot)
{
   gfx_widget_help_message_state_t* state   = &p_w_help_message_st;
   gfx_widget_help_message_slot_t* slot_ptr = state->slots[(size_t) slot];

   uintptr_t tag, timer_tag;

   if (!slot_ptr)
      return;

   tag        = (uintptr_t) slot_ptr;
   timer_tag  = (uintptr_t) &slot_ptr->timeout_timer;

   // Kill all animations and timers
   gfx_animation_kill_by_tag(&tag);
   gfx_animation_kill_by_tag(&timer_tag);

   /* Free next message strings, if any */
   if (slot_ptr->next.title)
      free(slot_ptr->next.title);

   if (slot_ptr->next.message)
      free(slot_ptr->next.message);

   free(slot_ptr);
   state->slots[(size_t) slot] = NULL;

   /* Decrement counter */
   state->active_slots--;
}

static bool gfx_widget_help_message_init(bool video_is_threaded, bool fullscreen)
{
   size_t i;

   gfx_widget_help_message_state_t* state = &p_w_help_message_st;

   /* Init state */
   memset(state, 0, sizeof(*state));

   return true;
}

static void gfx_widget_help_message_slot_layout(gfx_widget_help_message_slot_t* slot, float scale_factor)
{
   gfx_widget_help_message_state_t* state = &p_w_help_message_st;
   dispgfx_widget_t* p_dispwidget         = dispwidget_get_ptr();
   gfx_widget_font_data_t *font           = &p_dispwidget->gfx_widget_fonts.msg_queue;

   int lines, available_message_width, available_title_width, width, title_width;
   size_t i, title_chars_count, original_title_len;

   /* Width depends on slot */
   switch (slot->slot)
   {
      case HELP_MESSAGE_SLOT_TOP_LEFT:
      case HELP_MESSAGE_SLOT_TOP_RIGHT:
      case HELP_MESSAGE_SLOT_MIDDLE_RIGHT:
      case HELP_MESSAGE_SLOT_BOTTOM_RIGHT:
      case HELP_MESSAGE_SLOT_BOTTOM_LEFT:
      case HELP_MESSAGE_SLOT_MIDDLE_LEFT:
      default:
         width = 325;
         break;
      case HELP_MESSAGE_SLOT_TOP_MIDDLE:
      case HELP_MESSAGE_SLOT_BOTTOM_MIDDLE:
         width = 550;
         break;
   }

   slot->layout.width = (int) roundf((float) width * scale_factor);

   /* Generic layout */
   slot->layout.header_height = state->layout.header_height;

   /* Only count left padding for message, the wrapping uncertainty counts as the right padding */
   available_message_width = slot->layout.width - state->layout.message_padding_x;
   available_title_width   = slot->layout.width - state->layout.message_padding_x * 4 - state->layout.pellet_size;

   /* Truncate title */
   original_title_len   = strlen(slot->original_title);
   title_chars_count    = TITLE_MAX_LENGTH;
   title_width          = font_driver_get_message_width(font->font, slot->original_title, original_title_len, 1.0f);

   if (title_width > available_title_width)
   {
      /* Naive truncate using width / chars count ratio */
      float to_truncate = ((float) available_title_width) / ((float) title_width);
      title_chars_count = (size_t) roundf((float) original_title_len * to_truncate) - 1; /* -1 to account for uncertainty */
   }

   strncpy(slot->truncated_title, slot->original_title, MIN(TITLE_MAX_LENGTH, title_chars_count));

   /* Perform word wrapping */
   word_wrap(
      slot->wrapped_message,
      slot->original_message,
      (int) roundf((float) available_message_width / (float) state->layout.glyph_width),
      true,
      0
   );

   /* Count lines for height */
   lines = 1;
   for (i = 0; i < MESSAGE_MAX_LENGTH; i++)
   {
      if (slot->wrapped_message[i] == '\n')
         lines++;
      else if (slot->wrapped_message[i] == '\0')
         break;
   }

   slot->layout.message_height = state->layout.line_height * lines + state->layout.padding_y * 2;

   /* Positioning */
   switch (slot->slot)
   {
      default:
      case HELP_MESSAGE_SLOT_TOP_LEFT:
         slot->layout.x = 0;
         slot->layout.y = 0;
         break;
      case HELP_MESSAGE_SLOT_TOP_MIDDLE:
         slot->layout.x = state->layout.display_width / 2 - slot->layout.width / 2;
         slot->layout.y = 0;
         break;
      case HELP_MESSAGE_SLOT_TOP_RIGHT:
         slot->layout.x = state->layout.display_width - slot->layout.width;
         slot->layout.y = 0;
         break;
      case HELP_MESSAGE_SLOT_MIDDLE_RIGHT:
         slot->layout.x = state->layout.display_width - slot->layout.width;
         slot->layout.y = state->layout.display_height / 2 - (slot->layout.header_height + slot->layout.message_height) / 2;
         break;
      case HELP_MESSAGE_SLOT_BOTTOM_RIGHT:
         slot->layout.x = state->layout.display_width - slot->layout.width;
         slot->layout.y = state->layout.display_height - (slot->layout.header_height + slot->layout.message_height);
         break;
      case HELP_MESSAGE_SLOT_BOTTOM_MIDDLE:
         slot->layout.x = state->layout.display_width / 2 - slot->layout.width / 2;
         slot->layout.y = state->layout.display_height - (slot->layout.header_height + slot->layout.message_height);
         break;
      case HELP_MESSAGE_SLOT_BOTTOM_LEFT:
         slot->layout.x = 0;
         slot->layout.y = state->layout.display_height - (slot->layout.header_height + slot->layout.message_height);
         break;
      case HELP_MESSAGE_SLOT_MIDDLE_LEFT:
         slot->layout.x = 0;
         slot->layout.y = state->layout.display_height / 2 - (slot->layout.header_height + slot->layout.message_height) / 2;
         break;
   }
}

static void gfx_widget_help_message_layout(void *data, bool is_threaded, const char *dir_assets, char *font_path)
{
   size_t i;

   dispgfx_widget_t* p_dispwidget         = (dispgfx_widget_t*)data;
   gfx_widget_help_message_state_t* state = &p_w_help_message_st;
   gfx_widget_font_data_t *font           = &p_dispwidget->gfx_widget_fonts.msg_queue;

   float scale_factor = p_dispwidget->last_scale_factor;

   /* Messages layout */
   state->layout.display_width       = p_dispwidget->last_video_width;
   state->layout.display_height      = p_dispwidget->last_video_height;
   state->layout.glyph_width         = font_driver_get_message_width(font->font, "a", 1, 1.0f);
   state->layout.line_height         = font->line_height;
   state->layout.title_padding_x     = (int) roundf(35.0f * scale_factor);
   state->layout.message_padding_x   = (int) roundf(22.5f * scale_factor);
   state->layout.padding_y           = (int) roundf(15.0f * scale_factor);
   state->layout.header_height       = font->line_height + 2 * state->layout.padding_y;
   state->layout.pellet_size         = (int) roundf(15.0f * scale_factor);
   state->layout.pellet_offset_y     = state->layout.header_height / 2 - state->layout.pellet_size / 2;
   state->layout.title_offset_x      = state->layout.title_padding_x * 2 + state->layout.pellet_size;

   for (i = 0; i < _HELP_MESSAGE_SLOT_MAX; i++)
   {
      gfx_widget_help_message_slot_t* slot = state->slots[i];

      if (slot)
         gfx_widget_help_message_slot_layout(slot, scale_factor);
   }
}

static void gfx_widget_help_message_slot_frame(
      gfx_widget_help_message_slot_t* slot,
      const video_frame_info_t* video_info,
      dispgfx_widget_t* p_dispwidget)
{
   static float background_color[16] = COLOR_HEX_TO_FLOAT(0x000000, 0.9f);
   static float pellet_color[16]     = COLOR_HEX_TO_FLOAT(0x1FB318, 1.0f);

   static uint32_t title_color   = 0x1FB318FF;
   static uint32_t message_color = 0xFFFFFFFF;

   gfx_widget_help_message_state_t* state = &p_w_help_message_st;

   const unsigned video_width  = video_info->width;
   const unsigned video_height = video_info->height;

   int left_side, top_side, width, height;

   if (!slot)
      return;

   width       = slot->layout.width;
   height      = slot->layout.header_height + slot->layout.message_height;
   left_side   = slot->layout.x;
   top_side    = slot->layout.y;

   /* Positioning */
   switch(slot->slot)
   {
      /* Sliding from left side */
      case HELP_MESSAGE_SLOT_TOP_LEFT:
      case HELP_MESSAGE_SLOT_BOTTOM_LEFT:
      case HELP_MESSAGE_SLOT_MIDDLE_LEFT:
         left_side -= roundf((float) width * slot->slide_animation);
         break;
      /* Sliding from top side */
      case HELP_MESSAGE_SLOT_TOP_MIDDLE:
         top_side -= roundf((float) height * slot->slide_animation);
         break;
      /* Sliding from right side */
      case HELP_MESSAGE_SLOT_TOP_RIGHT:
      case HELP_MESSAGE_SLOT_MIDDLE_RIGHT:
      case HELP_MESSAGE_SLOT_BOTTOM_RIGHT:
         left_side += roundf((float) width * slot->slide_animation);
         break;
      /* Sliding from bottom side */
      case HELP_MESSAGE_SLOT_BOTTOM_MIDDLE:
         top_side += roundf((float) height * slot->slide_animation);
         break;
      default:
         break;
   }

   /* Background */
   gfx_display_draw_quad(
      video_info->userdata,
      video_width, video_height,
      left_side,
      top_side,
      width,
      height,
      video_width, video_height,
      background_color
   );

   /* Pellet */
   gfx_display_draw_quad(
      video_info->userdata,
      video_width, video_height,
      left_side + state->layout.title_padding_x,
      top_side + state->layout.pellet_offset_y,
      state->layout.pellet_size,
      state->layout.pellet_size,
      video_width, video_height,
      pellet_color
   );

   /* Title */
   gfx_widgets_draw_text(
      &p_dispwidget->gfx_widget_fonts.msg_queue,
      slot->truncated_title,
      left_side + state->layout.title_offset_x,
      top_side + slot->layout.header_height / 2 + p_dispwidget->gfx_widget_fonts.msg_queue.line_centre_offset,
      video_width, video_height,
      title_color,
      TEXT_ALIGN_LEFT,
      true
   );

   /* Message */
   gfx_widgets_draw_text(
      &p_dispwidget->gfx_widget_fonts.msg_queue,
      slot->wrapped_message,
      left_side + state->layout.message_padding_x,
      top_side + slot->layout.header_height + state->layout.padding_y + p_dispwidget->gfx_widget_fonts.msg_queue.line_centre_offset,
      video_width, video_height,
      message_color,
      TEXT_ALIGN_LEFT,
      true
   );
}

static void gfx_widget_help_message_frame(void* data, void* userdata)
{
   size_t i;

   gfx_widget_help_message_state_t* state = &p_w_help_message_st;
   const video_frame_info_t* video_info   = (const video_frame_info_t*)data;
   dispgfx_widget_t* p_dispwidget         = (dispgfx_widget_t*)userdata;

   if (state->active_slots == 0)
      return;

   for (i = 0; i < _HELP_MESSAGE_SLOT_MAX; i++)
   {
      gfx_widget_help_message_slot_t* slot = state->slots[i];

      if (slot)
         gfx_widget_help_message_slot_frame(slot, video_info, p_dispwidget);
   }
}

static void gfx_widget_help_message_free()
{
   size_t i;

   for (i = 0; i < _HELP_MESSAGE_SLOT_MAX; i++)
      gfx_widget_help_message_slot_release(i);
}

static void gfx_widget_help_message_on_slide_in(void* userdata)
{
   gfx_widget_help_message_slot_t* slot_ptr = (gfx_widget_help_message_slot_t*) userdata;

   slot_ptr->state = GFX_WIDGET_HELP_MESSAGE_STATE_IDLE;
}

static void gfx_widget_help_message_on_slide_out(void* userdata)
{
   gfx_widget_help_message_slot_t* slot_ptr = (gfx_widget_help_message_slot_t*) userdata;

   gfx_widget_help_message_slot_release(slot_ptr->slot);
}

static void gfx_widget_help_message_on_timeout(void* userdata)
{
   gfx_widget_help_message_slot_t* slot_ptr = (gfx_widget_help_message_slot_t*) userdata;

   gfx_animation_ctx_entry_t entry;

   entry.subject        = &slot_ptr->slide_animation;
   entry.cb             = gfx_widget_help_message_on_slide_out;
   entry.userdata       = slot_ptr;
   entry.tag            = (uintptr_t) slot_ptr;
   entry.duration       = MSG_QUEUE_ANIMATION_DURATION;
   entry.target_value   = 1.0f;
   entry.easing_enum    = EASING_OUT_QUAD;

   gfx_animation_push(&entry);

   slot_ptr->state = GFX_WIDGET_HELP_MESSAGE_STATE_SLIDING_OUT;
}

/* Public functions */

void gfx_widget_help_message_push(enum help_message_slot slot, const char* title, const char* message, bool animated, retro_time_t timeout)
{
   gfx_widget_help_message_state_t* state   = &p_w_help_message_st;
   gfx_widget_help_message_slot_t* slot_ptr = gfx_widget_help_message_slot_prepare(slot);
   dispgfx_widget_t* p_dispwidget           = dispwidget_get_ptr();

   /* Ensure we are pushing a valid help message */
   if (string_is_empty(title) || string_is_empty(message))
      return;

   /* Case 1: there is no message, add one */
   /* TODO: turn into a switch case to ensure every case is handled */
   if (slot_ptr->state == GFX_WIDGET_HELP_MESSAGE_STATE_NONE)
   {
      gfx_animation_ctx_entry_t entry;

      /* Generic attrs */
      slot_ptr->slot = slot;

      /* Copy texts */
      snprintf(slot_ptr->original_title, TITLE_MAX_LENGTH, "%s", title);
      snprintf(slot_ptr->original_message, MESSAGE_MAX_LENGTH, "%s", message);

      /* Layout */
      gfx_widget_help_message_slot_layout(slot_ptr, p_dispwidget->last_scale_factor);

      /* Reset slot state */
      slot_ptr->slide_animation = 1.0f;

      // Start animation
      entry.subject        = &slot_ptr->slide_animation;
      entry.cb             = gfx_widget_help_message_on_slide_in;
      entry.userdata       = slot_ptr;
      entry.tag            = (uintptr_t) slot_ptr;
      entry.duration       = MSG_QUEUE_ANIMATION_DURATION;
      entry.target_value   = 0.0f;
      entry.easing_enum    = EASING_OUT_QUAD;

      gfx_animation_push(&entry);

      /* Start timeout if necessary */
      if (timeout)
      {
         gfx_timer_ctx_entry_t timer_entry;

         timer_entry.duration = timeout;
         timer_entry.cb       = gfx_widget_help_message_on_timeout;
         timer_entry.userdata = slot_ptr;

         gfx_animation_timer_start(&slot_ptr->timeout_timer, &timer_entry);
      }

      /* Change state */
      slot_ptr->state = GFX_WIDGET_HELP_MESSAGE_STATE_SLIDING_IN;

      /* Increment counter */
      state->active_slots++;
   }
}

void gfx_widget_help_message_dismiss(enum help_message_slot slot, bool animated)
{
   // TODO: do it only if slot_ptr is valid
}

void gfx_widget_help_message_dismiss_all(bool animated)
{
   gfx_widget_help_message_state_t* state = &p_w_help_message_st;

   size_t i;

   if (state->active_slots == 0)
      return;

   for (i = 0; i < _HELP_MESSAGE_SLOT_MAX; i++)
      gfx_widget_help_message_dismiss(i, animated);
}

/* Widget struct */
const gfx_widget_t gfx_widget_help_message = {
   &gfx_widget_help_message_init,
   gfx_widget_help_message_free,
   NULL, /* context_reset */
   NULL, /* context_destroy */
   gfx_widget_help_message_layout,
   NULL, /* iterate */
   gfx_widget_help_message_frame,
};
