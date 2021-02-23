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

/* Max title length (before truncating!) */
#define TITLE_MAX_LENGTH 64 // TODO: ensure creating a widget with a longer title truncates it and doesn't corrupt anything

/* Max message length (after word wrapping!) */
#define MESSAGE_MAX_LENGTH 2048 // TODO: ensure creating a widget with a longer message truncates it and doesn't corrupt anything

enum gfx_widget_help_message_slot_state
{
   GFX_WIDGET_HELP_MESSAGE_STATE_SLIDING_IN = 0, // a message is currently sliding in
   GFX_WIDGET_HELP_MESSAGE_STATE_IDLE, // a message is visible and waiting to disappear or change its text
   GFX_WIDGET_HELP_MESSAGE_STATE_TEXT_CHANGING, // a message is visible and its text is currently changing
   GFX_WIDGET_HELP_MESSAGE_STATE_SLIDING_OUT, // a message is currently sliding out
};

/* Struct defining one help message */
typedef struct gfx_widget_help_message
{
   char original_title[TITLE_MAX_LENGTH]; /* original title, truncated into truncated_title */
   char original_message[MESSAGE_MAX_LENGTH]; /* original message, word wrapped into wrapped_message */

   char truncated_title[TITLE_MAX_LENGTH]; /* actually displayed title */
   char wrapped_message[MESSAGE_MAX_LENGTH]; /* actually displayed text */

   enum help_message_slot slot;

   struct
   {
      int x;
      int y;
      unsigned width;

      unsigned header_height;
      unsigned message_height;
   } layout;
} gfx_widget_help_message_t;

/* Single help message slot state */
typedef struct gfx_widget_help_message_slot
{
   enum gfx_widget_help_message_slot_state state;

   /* Goes from 1.0f to 0.0f, represents the slide in / out animation.
      Formula is x = x - (width * slide_animation) */
   float slide_animation;

   /* Currently displayed message */
   gfx_widget_help_message_t* current;

   /* Potential next message to show (for the transition) */
   gfx_widget_help_message_t* next;
} gfx_widget_help_message_slot_t;

/* Global help message "widget" state */
struct gfx_widget_help_message_state
{
   gfx_widget_help_message_slot_t* slots[_HELP_MESSAGE_SLOT_MAX];

   int messages_count; /* used to know if we should draw anything */

   struct {
      unsigned message_width;
      unsigned header_height;
      unsigned glyph_width;
      unsigned line_height;
      unsigned display_width;
      unsigned display_height;
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
   // TODO: slot init

   state->slots[(size_t) slot] = slot_ptr;

   return slot_ptr;
}

void gfx_widget_help_message_slot_release(enum help_message_slot slot)
{
   gfx_widget_help_message_state_t* state   = &p_w_help_message_st;
   gfx_widget_help_message_slot_t* slot_ptr = state->slots[(size_t) slot];

   if (!slot_ptr)
      return;

   // TODO: kill all animations and timers

   /* Free all messages */
   if (slot_ptr->current)
      free(slot_ptr->current);

   if (slot_ptr->next)
      free(slot_ptr->next);

   free(slot_ptr);
   state->slots[(size_t) slot] = NULL;
}

static bool gfx_widget_help_message_init(bool video_is_threaded, bool fullscreen)
{
   size_t i;

   gfx_widget_help_message_state_t* state = &p_w_help_message_st;

   /* Init state */
   memset(state, 0, sizeof(*state));

   return true;
}

static void gfx_widget_help_message_message_layout(gfx_widget_help_message_t* message)
{
   gfx_widget_help_message_state_t* state = &p_w_help_message_st;

   unsigned lines;
   size_t i;

   /* Universal layout */
   message->layout.width         = state->layout.message_width;
   message->layout.header_height = state->layout.header_height;

   /* TODO: Truncate title */
   strncpy(message->truncated_title, message->original_title, TITLE_MAX_LENGTH);

   /* Perform word wrapping */
   word_wrap(
      message->wrapped_message,
      message->original_message,
      state->layout.glyph_width,
      true,
      0
   );

   /* Count lines for height */
   lines = 1;
   for (i = 0; i < MESSAGE_MAX_LENGTH; i++)
   {
      if (message->wrapped_message[i] == '\n')
         lines++;
      else if (message->wrapped_message[i] == '\0')
         break;
   }

   message->layout.message_height = state->layout.line_height * lines; /* TODO: padding */

   /* Positioning */
   switch (message->slot)
   {
      default:
      case HELP_MESSAGE_SLOT_TOP_LEFT:
         message->layout.x = 0;
         message->layout.y = 0;
         break;
      case HELP_MESSAGE_SLOT_TOP_MIDDLE:
         message->layout.x = state->layout.display_width / 2 - message->layout.width / 2;
         message->layout.y = 0;
         break;
      case HELP_MESSAGE_SLOT_TOP_RIGHT:
         message->layout.x = state->layout.display_width - message->layout.width;
         message->layout.y = 0;
         break;
      case HELP_MESSAGE_SLOT_MIDDLE_RIGHT:
         message->layout.x = state->layout.display_width - message->layout.width;
         message->layout.y = state->layout.display_height / 2 - (message->layout.header_height + message->layout.message_height) / 2;
         break;
      case HELP_MESSAGE_SLOT_BOTTOM_RIGHT:
         message->layout.x = state->layout.display_width - message->layout.width;
         message->layout.y = state->layout.display_height - (message->layout.header_height + message->layout.message_height);
         break;
      case HELP_MESSAGE_SLOT_BOTTOM_MIDDLE:
         message->layout.x = state->layout.display_width / 2 - message->layout.width / 2;
         message->layout.y = state->layout.display_height - (message->layout.header_height + message->layout.message_height);
         break;
      case HELP_MESSAGE_SLOT_BOTTOM_LEFT:
         message->layout.x = 0;
         message->layout.y = state->layout.display_height - (message->layout.header_height + message->layout.message_height);
         break;
      case HELP_MESSAGE_SLOT_MIDDLE_LEFT:
         message->layout.x = 0;
         message->layout.y = state->layout.display_height / 2 - (message->layout.header_height + message->layout.message_height) / 2;
         break;
   }
}

static void gfx_widget_help_message_slot_layout(gfx_widget_help_message_slot_t* slot)
{
   if (slot->current)
      gfx_widget_help_message_message_layout(slot->current);

   if (slot->next)
      gfx_widget_help_message_message_layout(slot->next);
}

static void gfx_widget_help_message_layout(void *data, bool is_threaded, const char *dir_assets, char *font_path)
{
   size_t i;

   dispgfx_widget_t* p_dispwidget         = (dispgfx_widget_t*)data;
   gfx_widget_help_message_state_t* state = &p_w_help_message_st;
   gfx_widget_font_data_t *font_regular   = &p_dispwidget->gfx_widget_fonts.regular;

   // TODO: text size too? or does it scale automatically?

   /* Messages layout */
   state->layout.display_width  = p_dispwidget->last_video_width;
   state->layout.display_height = p_dispwidget->last_video_height;
   state->layout.message_width  = state->layout.display_width / 4;
   state->layout.header_height  = (unsigned) ((float) font_regular->line_height * 1.5f);
   state->layout.glyph_width    = font_driver_get_message_width(font_regular, "a", 1, 1.0f);
   state->layout.line_height    = font_regular->line_height;

   for (i = 0; i < _HELP_MESSAGE_SLOT_MAX; i++)
   {
      gfx_widget_help_message_slot_t* slot = state->slots[i];

      if (slot)
         gfx_widget_help_message_slot_layout(slot);
   }
}

static void gfx_widget_help_message_slot_frame(gfx_widget_help_message_slot_t* slot, const video_frame_info_t* video_info)
{
   static float header_color[16] = COLOR_HEX_TO_FLOAT(0x3A3A3A, 1.0f);
   static float body_color[16]   = COLOR_HEX_TO_FLOAT(0x7A7A7A, 1.0f);

   const unsigned video_width  = video_info->width;
   const unsigned video_height = video_info->height;

   int left_side, top_side;
   unsigned width, height;

   if (!slot->current)
      return;

   width  = slot->current->layout.width;
   height = slot->current->layout.header_height + slot->current->layout.message_height;

   left_side = slot->current->layout.x;
   top_side  = slot->current->layout.y;

   /* Positioning */
   switch(slot->current->slot)
   {
      /* Sliding from left side */
      case HELP_MESSAGE_SLOT_TOP_LEFT:
      case HELP_MESSAGE_SLOT_BOTTOM_LEFT:
      case HELP_MESSAGE_SLOT_MIDDLE_LEFT:
         left_side -= width * (int) roundf(slot->slide_animation);
         break;
      /* Sliding from top side */
      case HELP_MESSAGE_SLOT_TOP_MIDDLE:
         top_side -= height * (int) roundf(slot->slide_animation);
         break;
      /* Sliding from right side */
      case HELP_MESSAGE_SLOT_TOP_RIGHT:
      case HELP_MESSAGE_SLOT_MIDDLE_RIGHT:
      case HELP_MESSAGE_SLOT_BOTTOM_RIGHT:
         left_side += width * (int) roundf(slot->slide_animation);
         break;
      /* Sliding from bottom side */
      case HELP_MESSAGE_SLOT_BOTTOM_MIDDLE:
         top_side += height * (int) roundf(slot->slide_animation);
         break;
      default:
         break;
   }

   /* TODO: padding */

   /* Header */
   gfx_display_draw_quad(
      video_info->userdata,
      video_width,
      video_height,
      left_side,
      top_side,
      width,
      slot->current->layout.header_height,
      video_width,
      video_height,
      header_color
   );

   /* Body */
   gfx_display_draw_quad(
      video_info->userdata,
      video_width,
      video_height,
      left_side,
      top_side + slot->current->layout.header_height,
      width,
      slot->current->layout.message_height,
      video_width,
      video_height,
      body_color
   );

   /* TODO: text */
}

static void gfx_widget_help_message_frame(void* data, void* userdata)
{
   size_t i;

   gfx_widget_help_message_state_t* state = &p_w_help_message_st;
   const video_frame_info_t* video_info = (const video_frame_info_t*)data;

   if (state->messages_count == 0)
      return;

   for (i = 0; i < _HELP_MESSAGE_SLOT_MAX; i++)
   {
      gfx_widget_help_message_slot_t* slot = state->slots[i];

      if (slot)
         gfx_widget_help_message_slot_frame(slot, video_info);
   }
}

static void gfx_widget_help_message_free()
{
   size_t i;

   for (i = 0; i < _HELP_MESSAGE_SLOT_MAX; i++)
      gfx_widget_help_message_slot_release(i);
}

// TODO: free
gfx_widget_help_message_t* gfx_widgets_help_message_init(enum help_message_slot slot, const char* title, const char* message)
{
   gfx_widget_help_message_state_t* state = &p_w_help_message_st;
   gfx_widget_help_message_t* message_ptr = NULL;

   /* Init */
   message_ptr = calloc(1, sizeof(*message_ptr));

   if (!message_ptr)
      return NULL;

   /* Generic attrs */
   message_ptr->slot = slot;

   /* Copy texts */
   snprintf(message_ptr->original_title, TITLE_MAX_LENGTH, "%s", title);
   snprintf(message_ptr->original_message, MESSAGE_MAX_LENGTH, "%s", message);

   /* Layout */
   gfx_widget_help_message_message_layout(message_ptr);

   /* Increment counter */
   state->messages_count++;

   return message_ptr;
}

/* Public functions */

void gfx_widget_help_message_push(enum help_message_slot slot, const char* title, const char* message, bool animated, retro_time_t timeout)
{
   gfx_widget_help_message_slot_t* slot_ptr = gfx_widget_help_message_slot_prepare(slot);

   /* Case 1: there is no message, add one */
   if (!slot_ptr->current)
   {
      /* Add message */
      gfx_widget_help_message_t* message_ptr = gfx_widgets_help_message_init(slot, title, message);

      if (!message_ptr)
         return;

      slot_ptr->current = message_ptr;

      /* Reset slot state */
      slot_ptr->slide_animation = 1.0f;

      // TODO: Start animation
      slot_ptr->slide_animation = 0.0f; /* TODO: remove after animation is done */
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

   if (state->messages_count == 0)
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
