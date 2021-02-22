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

#include <string/stdstring.h>

/* Max title length (before truncating!) */
#define TITLE_MAX_LENGTH 64 // TODO: ensure creating a widget with a longer title truncates it and doesn't corrupt anything

/* Max message length (after word wrapping!) */
#define MESSAGE_MAX_LENGTH 2048 // TODO: ensure creating a widget with a longer message truncates it and doesn't corrupt anything

enum gfx_widget_help_message_slot_state
{
   GFX_WIDGET_HELP_MESSAGE_STATE_NONE = 0, // no message displayed in slot
   GFX_WIDGET_HELP_MESSAGE_STATE_SLIDING_IN, // a message is currently sliding in
   GFX_WIDGET_HELP_MESSAGE_STATE_IDLE, // a message is visible and waiting to disappear or change its text
   GFX_WIDGET_HELP_MESSAGE_STATE_TEXT_CHANGING, // a message is visible and its text is currently changing
   GFX_WIDGET_HELP_MESSAGE_STATE_SLIDING_OUT, // a message is currently sliding out
};

/* Struct defining one help message */
typedef struct gfx_widget_help_message
{
   const char original_title[TITLE_MAX_LENGTH]; /* original title, truncated into truncated_title */
   const char original_message[MESSAGE_MAX_LENGTH]; /* original message, word wrapped into wrapped_message */

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
   } metrics;
} gfx_widget_help_message_t;

/* Single help message slot state */
typedef struct gfx_widget_help_message_slot
{
   enum gfx_widget_help_message_slot_state state;

   /* Goes from 0.0f to 1.0f, represents the slide in / out position */
   float slide_animation;

   /* Currently displayed message */
   gfx_widget_help_message_t* current;

   /* Potential next message to show (for the transition) */
   gfx_widget_help_message_t* next;

#ifdef HAVE_THREADS
   slock_t* frame_lock;
#endif
} gfx_widget_help_message_slot_t;

/* Global help message "widget" state */
struct gfx_widget_help_message_state
{
   gfx_widget_help_message_slot_t* slots[_HELP_MESSAGE_SLOT_MAX];

   int messages_count; /* used to know if we should draw anything */
};

typedef struct gfx_widget_help_message_state gfx_widget_help_message_state_t;

static gfx_widget_help_message_state_t p_w_help_message_st;

static void gfx_widget_help_message_slot_init(gfx_widget_help_message_slot_t* slot)
{
#ifdef HAVE_THREADS
   slot->frame_lock = slock_new(); // TODO: free
#endif
}

static bool gfx_widget_help_message_init(bool video_is_threaded, bool fullscreen)
{
   size_t i;

   gfx_widget_help_message_state_t* state = &p_w_help_message_st;

   /* Init state */
   memset(state, 0, sizeof(*state));

   /* Init slots */
   for (i = 0; i < _HELP_MESSAGE_SLOT_MAX; i++)
   {
      gfx_widget_help_message_slot_t* slot = state->slots[i];

      gfx_widget_help_message_slot_init(slot);
   }

   return true;
}

static void gfx_widget_help_message_message_layout(
      gfx_widget_help_message_t* message,
      unsigned message_width,
      unsigned message_header_height,
      unsigned glyph_width,
      unsigned line_height,
      unsigned display_width,
      unsigned display_height)
{
   gfx_widget_help_message_state_t* state = &p_w_help_message_st;

   unsigned lines;
   size_t i;

   /* Universal metrics */
   message->metrics.width         = message_width;
   message->metrics.header_height = message_header_height;

   /* TODO: Truncate title */
   strncpy(message->truncated_title, message->original_title, TITLE_MAX_LENGTH);

   /* Perform word wrapping */
   word_wrap(
      message->wrapped_message,
      message->original_message,
      glyph_width,
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

   message->metrics.message_height = line_height * lines; /* TODO: padding */

   /* Positioning */
   switch (message->slot)
   {
      default:
      case HELP_MESSAGE_SLOT_TOP_LEFT:
         message->metrics.x = 0;
         message->metrics.y = 0;
         break;
      case HELP_MESSAGE_SLOT_TOP_MIDDLE:
         message->metrics.x = display_width / 2 - message->metrics.width / 2;
         message->metrics.y = 0;
         break;
      case HELP_MESSAGE_SLOT_TOP_RIGHT:
         message->metrics.x = display_width - message->metrics.width;
         message->metrics.y = 0;
         break;
      case HELP_MESSAGE_SLOT_MIDDLE_RIGHT:
         message->metrics.x = display_width - message->metrics.width;
         message->metrics.y = display_height / 2 - (message->metrics.header_height + message->metrics.message_height) / 2;
         break;
      case HELP_MESSAGE_SLOT_BOTTOM_RIGHT:
         message->metrics.x = display_width - message->metrics.width;
         message->metrics.y = display_height - (message->metrics.header_height + message->metrics.message_height);
         break;
      case HELP_MESSAGE_SLOT_BOTTOM_MIDDLE:
         message->metrics.x = display_width / 2 - message->metrics.width / 2;
         message->metrics.y = display_height - (message->metrics.header_height + message->metrics.message_height);
         break;
      case HELP_MESSAGE_SLOT_BOTTOM_LEFT:
         message->metrics.x = 0;
         message->metrics.y = display_height - (message->metrics.header_height + message->metrics.message_height);
         break;
      case HELP_MESSAGE_SLOT_MIDDLE_LEFT:
         message->metrics.x = 0;
         message->metrics.y = display_height / 2 - (message->metrics.header_height + message->metrics.message_height) / 2;
         break;
   }
}

static void gfx_widget_help_message_slot_layout(
      gfx_widget_help_message_slot_t* slot,
      unsigned message_width,
      unsigned message_header_height,
      unsigned glyph_width,
      unsigned line_height,
      unsigned display_width,
      unsigned display_height)
{
#ifdef HAVE_THREADS
   slock_lock(slot->frame_lock);
#endif

   if (slot->current)
      gfx_widget_help_message_message_layout(
         slot->current,
         message_width,
         message_header_height,
         glyph_width,
         line_height,
         display_width,
         display_height
      );

   if (slot->next)
      gfx_widget_help_message_message_layout(
         slot->next,
         message_width,
         message_header_height,
         glyph_width,
         line_height,
         display_width,
         display_height
      );

#ifdef HAVE_THREADS
   slock_unlock(slot->frame_lock);
#endif
}

static void gfx_widget_help_message_layout(void *data, bool is_threaded, const char *dir_assets, char *font_path)
{
   size_t i;

   dispgfx_widget_t* p_dispwidget         = (dispgfx_widget_t*)data;
   gfx_widget_help_message_state_t* state = &p_w_help_message_st;
   unsigned last_video_width              = p_dispwidget->last_video_width;
   unsigned last_video_height             = p_dispwidget->last_video_height;
   gfx_widget_font_data_t *font_regular   = &p_dispwidget->gfx_widget_fonts.regular;

   // TODO: text size too? or does it scale automatically?

   /* Messages metrics */
   unsigned message_width = last_video_width / 4;
   unsigned header_height = font_regular->line_height * 2;
   unsigned glyph_width   = font_driver_get_message_width(font_regular, "a", 1, 1.0f);
   unsigned line_height   = font_regular->line_height;

   for (i = 0; i < _HELP_MESSAGE_SLOT_MAX; i++)
   {
      gfx_widget_help_message_slot_t* slot = state->slots[i];

      gfx_widget_help_message_slot_layout(
         slot,
         message_width,
         header_height,
         glyph_width,
         line_height,
         last_video_width,
         last_video_height
      );
   }
}

static void gfx_widget_help_message_slot_frame(gfx_widget_help_message_slot_t* slot)
{
#ifdef HAVE_THREADS
   slock_lock(slot->frame_lock);
#endif

   if (slot->state == GFX_WIDGET_HELP_MESSAGE_STATE_NONE)
      goto end;

   /* Draw current message */
   // TODO: do it

end:
#ifdef HAVE_THREADS
   slock_unlock(slot->frame_lock);
#endif
}

static void gfx_widget_help_message_frame(void* data, void* userdata)
{
   size_t i;

   gfx_widget_help_message_state_t* state = &p_w_help_message_st;

   if (state->messages_count == 0)
      return;

   for (i = 0; i < _HELP_MESSAGE_SLOT_MAX; i++)
   {
      gfx_widget_help_message_slot_t* slot = state->slots[i];

      gfx_widget_help_message_slot_frame(slot);
   }
}

const gfx_widget_t gfx_widget_help_message = {
   &gfx_widget_help_message_init,
   NULL, /* free */
   NULL, /* context_reset */
   NULL, /* context_destroy */
   gfx_widget_help_message_layout,
   NULL, /* iterate */
   gfx_widget_help_message_frame,
};
