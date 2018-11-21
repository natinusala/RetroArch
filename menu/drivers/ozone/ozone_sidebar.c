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

/* TODO Fluid sidebar width ? */

#include "ozone.h"
#include "ozone_theme.h"
#include "ozone_display.h"
#include "ozone_sidebar.h"
#include "ozone_metrics.h"

#include <string/stdstring.h>
#include <file/file_path.h>
#include <streams/file_stream.h>

#include "../../menu_animation.h"

#include "../../../configuration.h"

enum msg_hash_enums ozone_system_tabs_value[OZONE_SYSTEM_TAB_LAST] = {
   MENU_ENUM_LABEL_VALUE_MAIN_MENU,
   MENU_ENUM_LABEL_VALUE_SETTINGS_TAB,
   MENU_ENUM_LABEL_VALUE_HISTORY_TAB,
   MENU_ENUM_LABEL_VALUE_FAVORITES_TAB,
   MENU_ENUM_LABEL_VALUE_MUSIC_TAB,
#if defined(HAVE_FFMPEG) || defined(HAVE_MPV)
   MENU_ENUM_LABEL_VALUE_VIDEO_TAB,
#endif
#ifdef HAVE_IMAGEVIEWER
   MENU_ENUM_LABEL_VALUE_IMAGES_TAB,
#endif
#ifdef HAVE_NETWORKING
   MENU_ENUM_LABEL_VALUE_NETPLAY_TAB,
#endif
   MENU_ENUM_LABEL_VALUE_ADD_TAB
};

enum menu_settings_type ozone_system_tabs_type[OZONE_SYSTEM_TAB_LAST] = {
   MENU_SETTINGS,
   MENU_SETTINGS_TAB,
   MENU_HISTORY_TAB,
   MENU_FAVORITES_TAB,
   MENU_MUSIC_TAB,
#if defined(HAVE_FFMPEG) || defined(HAVE_MPV)
   MENU_VIDEO_TAB,
#endif
#ifdef HAVE_IMAGEVIEWER
   MENU_IMAGES_TAB,
#endif
#ifdef HAVE_NETWORKING
   MENU_NETPLAY_TAB,
#endif
   MENU_ADD_TAB
};

enum msg_hash_enums ozone_system_tabs_idx[OZONE_SYSTEM_TAB_LAST] = {
   MENU_ENUM_LABEL_MAIN_MENU,
   MENU_ENUM_LABEL_SETTINGS_TAB,
   MENU_ENUM_LABEL_HISTORY_TAB,
   MENU_ENUM_LABEL_FAVORITES_TAB,
   MENU_ENUM_LABEL_MUSIC_TAB,
#if defined(HAVE_FFMPEG) || defined(HAVE_MPV)
   MENU_ENUM_LABEL_VIDEO_TAB,
#endif
#ifdef HAVE_IMAGEVIEWER
   MENU_ENUM_LABEL_IMAGES_TAB,
#endif
#ifdef HAVE_NETWORKING
   MENU_ENUM_LABEL_NETPLAY_TAB,
#endif
   MENU_ENUM_LABEL_ADD_TAB
};

unsigned ozone_system_tabs_icons[OZONE_SYSTEM_TAB_LAST] = {
   OZONE_TAB_TEXTURE_MAIN_MENU,
   OZONE_TAB_TEXTURE_SETTINGS,
   OZONE_TAB_TEXTURE_HISTORY,
   OZONE_TAB_TEXTURE_FAVORITES,
   OZONE_TAB_TEXTURE_MUSIC,
#if defined(HAVE_FFMPEG) || defined(HAVE_MPV)
   OZONE_TAB_TEXTURE_VIDEO,
#endif
#ifdef HAVE_IMAGEVIEWER
   OZONE_TAB_TEXTURE_IMAGE,
#endif
#ifdef HAVE_NETWORKING
   OZONE_TAB_TEXTURE_NETWORK,
#endif
   OZONE_TAB_TEXTURE_SCAN_CONTENT
};

void ozone_draw_sidebar(ozone_handle_t *ozone, video_frame_info_t *video_info)
{
   size_t y;
   unsigned i, sidebar_height;
   char console_title[255];
   menu_animation_ctx_ticker_t ticker;

   unsigned selection_y          = 0;
   unsigned selection_old_y      = 0;
   unsigned horizontal_list_size = 0;

   if (!ozone->draw_sidebar)
      return;

   if (ozone->horizontal_list)
      horizontal_list_size = ozone->horizontal_list->size;

   menu_display_scissor_begin(video_info, 0, 87, 408, video_info->height - 87 - 78);

   /* Background */
   sidebar_height = video_info->height - 87 - 55 - 78;

   if (!video_info->libretro_running)
   {
      menu_display_draw_quad(video_info, ozone->sidebar_offset, 88, 408, 55/2, video_info->width, video_info->height, ozone->theme->sidebar_top_gradient);
      menu_display_draw_quad(video_info, ozone->sidebar_offset, 88 + 55/2, 408, sidebar_height, video_info->width, video_info->height, ozone->theme->sidebar_background);
      menu_display_draw_quad(video_info, ozone->sidebar_offset, 55*2 + sidebar_height, 408, 55/2 + 1, video_info->width, video_info->height, ozone->theme->sidebar_bottom_gradient);
   }

   /* Tabs */
   /* y offset computation */
   y = ozone->metrics.sidebar.start_y;
   for (i = 0; i < ozone->system_tab_end + horizontal_list_size + 1; i++)
   {
      if (i == ozone->categories_selection_ptr)
      {
         selection_y = y;
         if (ozone->categories_selection_ptr > ozone->system_tab_end)
            selection_y += 30;
      }

      if (i == ozone->categories_active_idx_old)
      {
         selection_old_y = y;
         if (ozone->categories_active_idx_old > ozone->system_tab_end)
            selection_old_y += 30;
      }

      y += 65;
   }

   /* Cursor */
   if (ozone->cursor_in_sidebar)
      ozone_draw_cursor(ozone, video_info, ozone->sidebar_offset + 41, 408 - 81, 52, selection_y-8 + ozone->animations.scroll_y_sidebar, ozone->animations.cursor_alpha);

   if (ozone->cursor_in_sidebar_old)
      ozone_draw_cursor(ozone, video_info, ozone->sidebar_offset + 41, 408 - 81, 52, selection_old_y-8 + ozone->animations.scroll_y_sidebar, 1-ozone->animations.cursor_alpha);

   /* Menu tabs */
   y = ozone->metrics.sidebar.start_y;
   menu_display_blend_begin(video_info);

   for (i = 0; i < ozone->system_tab_end+1; i++)
   {
      enum msg_hash_enums value_idx;
      const char *title = NULL;
      bool     selected = (ozone->categories_selection_ptr == i);
      unsigned     icon = ozone_system_tabs_icons[ozone->tabs[i]];

      /* Icon */
      ozone_draw_icon(video_info, 40, 40, ozone->tab_textures[icon], ozone->sidebar_offset + 41 + 10, y - 5 + ozone->animations.scroll_y_sidebar, video_info->width, video_info->height, 0, 1, (selected ? ozone->theme->text_selected : ozone->theme->entries_icon));

      value_idx = ozone_system_tabs_value[ozone->tabs[i]];
      title     = msg_hash_to_str(value_idx);

      /* Text */
      ozone_draw_text(video_info, ozone, title, ozone->sidebar_offset + 115 - 10, y + FONT_SIZE_SIDEBAR + ozone->animations.scroll_y_sidebar, TEXT_ALIGN_LEFT, video_info->width, video_info->height, ozone->fonts.sidebar, (selected ? ozone->theme->text_selected_rgba : ozone->theme->text_rgba), true);

      y += 65;
   }

   menu_display_blend_end(video_info);

   /* Console tabs */
   if (horizontal_list_size > 0)
   {
      menu_display_draw_quad(video_info, ozone->sidebar_offset + 41 + 10, y - 5 + ozone->animations.scroll_y_sidebar, 408-81, 1, video_info->width, video_info->height, ozone->theme->entries_border);

      y += 30;

      menu_display_blend_begin(video_info);

      for (i = 0; i < horizontal_list_size; i++)
      {
         bool selected = (ozone->categories_selection_ptr == ozone->system_tab_end + 1 + i);

         ozone_node_t *node = (ozone_node_t*) file_list_get_userdata_at_offset(ozone->horizontal_list, i);

         if (!node)
            goto console_iterate;

         /* Icon */
         ozone_draw_icon(video_info, 40, 40, node->icon, ozone->sidebar_offset + 41 + 10, y - 5 + ozone->animations.scroll_y_sidebar, video_info->width, video_info->height, 0, 1, (selected ? ozone->theme->text_selected : ozone->theme->entries_icon));

         /* Text */
         ticker.idx        = ozone->frame_count / 20;
         ticker.len        = 19;
         ticker.s          = console_title;
         ticker.selected   = selected;
         ticker.str        = node->console_name;

         menu_animation_ticker(&ticker);

         ozone_draw_text(video_info, ozone, console_title, ozone->sidebar_offset + 115 - 10, y + FONT_SIZE_SIDEBAR + ozone->animations.scroll_y_sidebar, TEXT_ALIGN_LEFT, video_info->width, video_info->height, ozone->fonts.sidebar, (selected ? ozone->theme->text_selected_rgba : ozone->theme->text_rgba), true);

console_iterate:
         y += 65;
      }

      menu_display_blend_end(video_info);

   }

   font_driver_flush(video_info->width, video_info->height, ozone->fonts.sidebar, video_info);
   ozone->raster_blocks.sidebar.carr.coords.vertices = 0;

   menu_display_scissor_end(video_info);
}

void ozone_go_to_sidebar(ozone_handle_t *ozone, uintptr_t tag)
{
   struct menu_animation_ctx_entry entry;

   ozone->selection_old           = ozone->selection;
   ozone->cursor_in_sidebar_old   = ozone->cursor_in_sidebar;
   ozone->cursor_in_sidebar       = true;
   
   /* Cursor animation */
   ozone->animations.cursor_alpha = 0.0f;

   entry.cb = NULL;
   entry.duration = ANIMATION_CURSOR_DURATION;
   entry.easing_enum = EASING_OUT_QUAD;
   entry.subject = &ozone->animations.cursor_alpha;
   entry.tag = tag;
   entry.target_value = 1.0f;
   entry.userdata = NULL;

   menu_animation_push(&entry);
}

void ozone_leave_sidebar(ozone_handle_t *ozone, uintptr_t tag)
{
   struct menu_animation_ctx_entry entry;

   if (ozone->empty_playlist)
      return;

   ozone->categories_active_idx_old = ozone->categories_selection_ptr;
   ozone->cursor_in_sidebar_old     = ozone->cursor_in_sidebar;
   ozone->cursor_in_sidebar         = false;
   
   /* Cursor animation */
   ozone->animations.cursor_alpha   = 0.0f;

   entry.cb = NULL;
   entry.duration = ANIMATION_CURSOR_DURATION;
   entry.easing_enum = EASING_OUT_QUAD;
   entry.subject = &ozone->animations.cursor_alpha;
   entry.tag = tag;
   entry.target_value = 1.0f;
   entry.userdata = NULL;

   menu_animation_push(&entry);
}

unsigned ozone_get_selected_sidebar_y_position(ozone_handle_t *ozone)
{
   return ozone->categories_selection_ptr * 65 + (ozone->categories_selection_ptr > ozone->system_tab_end ? 30 : 0);
}

unsigned ozone_get_sidebar_height(ozone_handle_t *ozone)
{
   return (ozone->system_tab_end + 1 + (ozone->horizontal_list ? ozone->horizontal_list->size : 0)) * 65
      + (ozone->horizontal_list && ozone->horizontal_list->size > 0 ? 30 : 0);
}

void ozone_sidebar_goto(ozone_handle_t *ozone, unsigned new_selection)
{
   unsigned video_info_height;

   video_driver_get_size(NULL, &video_info_height);

   struct menu_animation_ctx_entry entry;

   menu_animation_ctx_tag tag = (uintptr_t)ozone;

   if (ozone->categories_selection_ptr != new_selection)
   {
      ozone->categories_active_idx_old = ozone->categories_selection_ptr;
      ozone->categories_selection_ptr = new_selection;

      ozone->cursor_in_sidebar_old = ozone->cursor_in_sidebar;

      menu_animation_kill_by_tag(&tag);
   }

   /* Cursor animation */
   ozone->animations.cursor_alpha = 0.0f;

   entry.cb = NULL;
   entry.duration = ANIMATION_CURSOR_DURATION;
   entry.easing_enum = EASING_OUT_QUAD;
   entry.subject = &ozone->animations.cursor_alpha;
   entry.tag = tag;
   entry.target_value = 1.0f;
   entry.userdata = NULL;

   menu_animation_push(&entry);

   /* Scroll animation */
   float new_scroll                             = 0;
   float selected_position_y                    = ozone_get_selected_sidebar_y_position(ozone);
   float current_selection_middle_onscreen      = ozone->metrics.sidebar.start_y + ozone->animations.scroll_y_sidebar + selected_position_y + 65 / 2;
   float bottom_boundary                        = video_info_height - 87 - 78;
   float entries_middle                         = video_info_height/2;
   float entries_height                         = ozone_get_sidebar_height(ozone);

   if (current_selection_middle_onscreen != entries_middle)
      new_scroll = ozone->animations.scroll_y_sidebar - (current_selection_middle_onscreen - entries_middle);

   if (new_scroll + entries_height < bottom_boundary)
      new_scroll = -(30 + entries_height - bottom_boundary);

   if (new_scroll > 0)
      new_scroll = 0;

   entry.cb = NULL;
   entry.duration = ANIMATION_CURSOR_DURATION;
   entry.easing_enum = EASING_OUT_QUAD;
   entry.subject = &ozone->animations.scroll_y_sidebar;
   entry.tag = tag;
   entry.target_value = new_scroll;
   entry.userdata = NULL;

   menu_animation_push(&entry);

   if (new_selection > ozone->system_tab_end)
   {
      ozone_change_tab(ozone, MENU_ENUM_LABEL_HORIZONTAL_MENU, MENU_SETTING_HORIZONTAL_MENU);
   }
   else
   {
      ozone_change_tab(ozone, ozone_system_tabs_idx[ozone->tabs[new_selection]], ozone_system_tabs_type[ozone->tabs[new_selection]]);
   }
}


void ozone_change_tab(ozone_handle_t *ozone, 
      enum msg_hash_enums tab, 
      enum menu_settings_type type)
{
   file_list_t *menu_stack = menu_entries_get_menu_stack_ptr(0);
   size_t stack_size;
   menu_ctx_list_t list_info;
   file_list_t *selection_buf = menu_entries_get_selection_buf_ptr(0);
   size_t selection = menu_navigation_get_selection();
   menu_file_list_cbs_t *cbs = selection_buf ?
      (menu_file_list_cbs_t*)file_list_get_actiondata_at_offset(selection_buf,
            selection) : NULL;

   list_info.type = MENU_LIST_HORIZONTAL;
   list_info.action = MENU_ACTION_LEFT;

   stack_size = menu_stack->size;

   if (menu_stack->list[stack_size - 1].label)
      free(menu_stack->list[stack_size - 1].label);
   menu_stack->list[stack_size - 1].label = NULL;
   
   menu_stack->list[stack_size - 1].label =
      strdup(msg_hash_to_str(tab));
   menu_stack->list[stack_size - 1].type =
      type;

   menu_driver_list_cache(&list_info);

   if (cbs && cbs->action_content_list_switch)
      cbs->action_content_list_switch(selection_buf, menu_stack, "", "", 0);
}

void ozone_init_horizontal_list(ozone_handle_t *ozone)
{
   menu_displaylist_info_t info;
   settings_t *settings         = config_get_ptr();

   menu_displaylist_info_init(&info);

   info.list                    = ozone->horizontal_list;
   info.path                    = strdup(
         settings->paths.directory_playlist);
   info.label                   = strdup(
         msg_hash_to_str(MENU_ENUM_LABEL_CONTENT_COLLECTION_LIST));
   info.exts                    = strdup(
         file_path_str(FILE_PATH_LPL_EXTENSION_NO_DOT));
   info.type_default            = FILE_TYPE_PLAIN;
   info.enum_idx                = MENU_ENUM_LABEL_CONTENT_COLLECTION_LIST;

   if (settings->bools.menu_content_show_playlists && !string_is_empty(info.path))
   {
      if (menu_displaylist_ctl(DISPLAYLIST_DATABASE_PLAYLISTS_HORIZONTAL, &info))
      {
         size_t i;
         for (i = 0; i < ozone->horizontal_list->size; i++)
         {
            ozone_node_t *node = ozone_alloc_node();
            file_list_set_userdata(ozone->horizontal_list, i, node);
         }

         menu_displaylist_process(&info);
      }
   }

   menu_displaylist_info_free(&info);
}

void ozone_refresh_horizontal_list(ozone_handle_t *ozone)
{
   ozone_context_destroy_horizontal_list(ozone);
   if (ozone->horizontal_list)
   {
      ozone_free_list_nodes(ozone->horizontal_list, false);
      file_list_free(ozone->horizontal_list);
   }
   ozone->horizontal_list = NULL;

   menu_driver_ctl(RARCH_MENU_CTL_SET_PREVENT_POPULATE, NULL);

   ozone->horizontal_list         = (file_list_t*)
      calloc(1, sizeof(file_list_t));

   if (ozone->horizontal_list)
      ozone_init_horizontal_list(ozone);

   ozone_context_reset_horizontal_list(ozone);
}

void ozone_context_reset_horizontal_list(ozone_handle_t *ozone)
{
   unsigned i;
   const char *title;
   char title_noext[255];

   size_t list_size  = ozone_list_get_size(ozone, MENU_LIST_HORIZONTAL);

   for (i = 0; i < list_size; i++)
   {
      const char *path     = NULL;
      ozone_node_t *node   = (ozone_node_t*)file_list_get_userdata_at_offset(ozone->horizontal_list, i);

      if (!node)
      {
         node = ozone_alloc_node();
         if (!node)
            continue;
      }

      file_list_get_at_offset(ozone->horizontal_list, i,
            &path, NULL, NULL, NULL);

      if (!path)
         continue;

      if (!strstr(path, file_path_str(FILE_PATH_LPL_EXTENSION)))
         continue;

      {
         struct texture_image ti;
         char *sysname             = (char*)
            malloc(PATH_MAX_LENGTH * sizeof(char));
         char *texturepath         = (char*)
            malloc(PATH_MAX_LENGTH * sizeof(char));
         char *content_texturepath = (char*)
            malloc(PATH_MAX_LENGTH * sizeof(char));
         char *icons_path          = (char*)
            malloc(PATH_MAX_LENGTH * sizeof(char));

         strlcpy(icons_path, ozone->icons_path, PATH_MAX_LENGTH * sizeof(char));

         sysname[0] = texturepath[0] = content_texturepath[0] = '\0';

         fill_pathname_base_noext(sysname, path,
               PATH_MAX_LENGTH * sizeof(char));

         fill_pathname_join_concat(texturepath, icons_path, sysname,
               file_path_str(FILE_PATH_PNG_EXTENSION),
               PATH_MAX_LENGTH * sizeof(char));

         /* If the playlist icon doesn't exist return default */

         if (!filestream_exists(texturepath))
               fill_pathname_join_concat(texturepath, icons_path, "default",
               file_path_str(FILE_PATH_PNG_EXTENSION),
               PATH_MAX_LENGTH * sizeof(char));

         ti.width         = 0;
         ti.height        = 0;
         ti.pixels        = NULL;
         ti.supports_rgba = video_driver_supports_rgba();

         if (image_texture_load(&ti, texturepath))
         {
            if(ti.pixels)
            {
               video_driver_texture_unload(&node->icon);
               video_driver_texture_load(&ti,
                     TEXTURE_FILTER_MIPMAP_LINEAR, &node->icon);
            }

            image_texture_free(&ti);
         }

         fill_pathname_join_delim(sysname, sysname,
               file_path_str(FILE_PATH_CONTENT_BASENAME), '-',
               PATH_MAX_LENGTH * sizeof(char));
         strlcat(content_texturepath, icons_path, PATH_MAX_LENGTH * sizeof(char));

         strlcat(content_texturepath, path_default_slash(), PATH_MAX_LENGTH * sizeof(char));
         strlcat(content_texturepath, sysname, PATH_MAX_LENGTH * sizeof(char));

         /* If the content icon doesn't exist return default-content */
         if (!filestream_exists(content_texturepath))
         {
            strlcat(icons_path, path_default_slash(), PATH_MAX_LENGTH * sizeof(char));
            strlcat(icons_path, "default", PATH_MAX_LENGTH * sizeof(char));
            fill_pathname_join_delim(content_texturepath, icons_path,
                  file_path_str(FILE_PATH_CONTENT_BASENAME), '-',
                  PATH_MAX_LENGTH * sizeof(char));
         }

         if (image_texture_load(&ti, content_texturepath))
         {
            if(ti.pixels)
            {
               video_driver_texture_unload(&node->content_icon);
               video_driver_texture_load(&ti,
                     TEXTURE_FILTER_MIPMAP_LINEAR, &node->content_icon);
            }

            image_texture_free(&ti);
         }

         /* Console name */
         menu_entries_get_at_offset(
            ozone->horizontal_list,
            i,
            &title, NULL, NULL, NULL, NULL);

         fill_pathname_base_noext(title_noext, title, sizeof(title_noext));

         /* Format : "Vendor - Console"
            Remove everything before the hyphen
            and the subsequent space */
         char *chr         = title_noext;
         bool hyphen_found = false;

         while (true)
         {
            if (*chr == '-')
            {
               hyphen_found = true;
               break;
            }
            else if (*chr == '\0')
               break;

            chr++;
         }

         if (hyphen_found)
            chr += 2;
         else
            chr = title_noext;

         node->console_name = strdup(chr);

         free(sysname);
         free(texturepath);
         free(content_texturepath);
         free(icons_path);
      }
   }
}

void ozone_context_destroy_horizontal_list(ozone_handle_t *ozone)
{
   unsigned i;
   size_t list_size = ozone_list_get_size(ozone, MENU_LIST_HORIZONTAL);

   for (i = 0; i < list_size; i++)
   {
      const char *path = NULL;
      ozone_node_t *node = (ozone_node_t*)file_list_get_userdata_at_offset(ozone->horizontal_list, i);

      if (!node)
         continue;

      file_list_get_at_offset(ozone->horizontal_list, i,
            &path, NULL, NULL, NULL);

      if (!path || !strstr(path, file_path_str(FILE_PATH_LPL_EXTENSION)))
         continue;

      video_driver_texture_unload(&node->icon);
      video_driver_texture_unload(&node->content_icon);
   }
}

bool ozone_is_playlist(ozone_handle_t *ozone)
{
   bool is_playlist;

   switch (ozone->categories_selection_ptr)
   {
      case OZONE_SYSTEM_TAB_MAIN:
      case OZONE_SYSTEM_TAB_SETTINGS:
      case OZONE_SYSTEM_TAB_ADD:
         is_playlist = false;
         break;
      case OZONE_SYSTEM_TAB_HISTORY:
      case OZONE_SYSTEM_TAB_FAVORITES:
      case OZONE_SYSTEM_TAB_MUSIC:
#if defined(HAVE_FFMPEG) || defined(HAVE_MPV)
      case OZONE_SYSTEM_TAB_VIDEO:
#endif
#ifdef HAVE_IMAGEVIEWER
      case OZONE_SYSTEM_TAB_IMAGES:
#endif
#ifdef HAVE_NETWORKING
      case OZONE_SYSTEM_TAB_NETPLAY:
#endif
      default:
         is_playlist = true;
         break;
   }

   return is_playlist && ozone->depth == 1;
}