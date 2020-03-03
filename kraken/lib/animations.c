/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2020 - natinusala
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

#include <stdlib.h>

#include "../kraken_lib.h"
#include "../kraken.h"

#include "../../verbosity.h"

#include "c/animations.inc.h"

#include "../../gfx/gfx_animation.h"

/* TODO:
         Either keep track of the animations somewhere and cancel them all when kraken gets unloaded
         or change the kraken deinit to have it deinit on exit and not on content load (probably better)
*/

typedef struct kraken_animations_userdata
{
   float value;   /* the animation value (the gfx_animation subject) */
   char* subject; /* the name of the Lua variable (is set to value every frame) */
   char* cb;      /* Lua global function to be executed after the animation is finished */
} kraken_animations_userdata_t;

static void kraken_animations_cb(void* userdata)
{
   kraken_animations_userdata_t* animation_userdata = (kraken_animations_userdata_t*) userdata;

   /* Run cb function if any */
   if (animation_userdata->cb)
   {
      lua_State* state = kraken_get_state();
      lua_getglobal(state, animation_userdata->cb);

      if (lua_pcall(state, 0, 0, 0))
         RARCH_ERR("[Kraken]: Error while calling kraken_animations_cb: %s\n", kraken_get_error(state));
   }

   /* Free userdata */
   free(animation_userdata->subject);

   if (animation_userdata->cb)
      free(animation_userdata->cb);

   free(animation_userdata);
}

static void kraken_animations_tick(void* userdata)
{
   kraken_animations_userdata_t* animation_userdata = (kraken_animations_userdata_t*) userdata;

   /* Update the Lua global, set the value in userdata->value */
   lua_State* state = kraken_get_state();
   lua_pushnumber(state, animation_userdata->value);
   lua_setglobal(state, animation_userdata->subject);
}

/* animations.push(
      subject: string,
      target_value: number,
      duration: integer,
      easing: integer,
      cb : string
   )
*/
static int kraken_animations_push(lua_State* state)
{
   int argc = lua_gettop(state);
   if (
      argc != 5 ||
      !lua_isstring(state, 1) ||                         //subject
      !lua_isnumber(state, 2) ||                         //target_value
      !lua_isinteger(state, 3) ||                        //duration
      !lua_isinteger(state, 4) ||                        //easing
      !(lua_isstring(state, 5) || lua_isnil(state, 5))   //cb
   )
   {
      RARCH_ERR("[Kraken]: animations.push: invalid arguments\n");
      return 0;
   }

   const char* subject = lua_tostring(state, 1);

   /* Ensure subject is a number, get initial value */
   lua_getglobal(state, subject);
   if (!lua_isnumber(state, -1))
   {
      RARCH_ERR("[Kraken]: animations.push: subject \"%s\" doesn't exist, is not global or is not a number\n", subject);
      return 0;
   }

   float initial_value = (float)lua_tonumber(state, -1);

   lua_pop(state, 1);

   /* Ensure cb is a function or nil */
   const char* cb = lua_isnil(state, 5) ? NULL : lua_tostring(state, 5);
   if (cb)
   {
      lua_getglobal(state, cb);
      if (!(lua_isfunction(state, -1) || lua_iscfunction(state, -1)))
      {
         RARCH_ERR("[Kraken]: animations.push: callback doesn't exist, is not global or is not a valid function\n", subject);
         return 0;
      }
      lua_pop(state, 1);
   }

   /* Prepare animation userdata */
   kraken_animations_userdata_t* userdata = (kraken_animations_userdata_t*) calloc(1, sizeof(*userdata));
   userdata->value   = initial_value;
   userdata->subject = strdup(subject);
   userdata->cb      = cb ? strdup(cb) : NULL;

   float target_value   = (float) lua_tonumber(state, 2);
   float duration       = (float) lua_tointeger(state, 3);
   int easing           = lua_tointeger(state, 4);

   /* Push animation */
   gfx_animation_ctx_entry_t entry;
   entry.easing_enum    = easing;
   entry.tag            = (uintptr_t) NULL;
   entry.duration       = duration;
   entry.target_value   = target_value;
   entry.subject        = &userdata->value;
   entry.cb             = kraken_animations_cb;
   entry.userdata       = userdata;
   entry.tick           = kraken_animations_tick;

   gfx_animation_push(&entry);

   return 0;
}

static void kraken_animations_register(lua_State* state)
{
   lua_register(state, "animations_push", kraken_animations_push);
}

kraken_module_t kraken_module_animations = {
   "animations",
   animations_lua,
   sizeof(animations_lua),
   kraken_animations_register
};
