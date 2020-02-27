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

#include "lichen.h"

#include <stdlib.h>

#include "../verbosity.h"

#include "lichen_lua_user.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#ifdef HAVE_THREADS
#include <rthreads/rthreads.h>

static uintptr_t mainThreadId;
static lua_State *videoThreadState;
#endif

static lua_State *mainThreadState;

bool lichen_init(void)
{
   /* TODO: don't do anything if there are no addons */

   /* Safety check */
   if (mainThreadState)
   {
      RARCH_LOG("[Lichen]: Already initialized\n");
      return true;
   }

   /* Create state */
#ifdef HAVE_THREADS
   lichen_init_lock();
#endif
   mainThreadState = luaL_newstate();

   if (!mainThreadState)
   {
      RARCH_ERR("[Lichen]: Failed to initialize main thread state\n");
      return false;
   }

   /* Load standard libs and RetroArch API */
   luaL_openlibs(mainThreadState);
   lichen_register_functions(mainThreadState);

   /* Load RetroArch API modules */
   /* TODO: do it */

   /* Load addons */
   /* TODO: properly do it */
   if (luaL_dofile(mainThreadState, "test.lua"))
      RARCH_ERR("[Lichen]: Unable to load test.lua\n");

   /* Spawn video thread if needed */
#ifdef HAVE_THREADS
   mainThreadId      = sthread_get_current_thread_id();
   videoThreadState  = lua_newthread(mainThreadState);

   if (!videoThreadState)
   {
      RARCH_ERR("[Lichen]: Failed to initialize video thread state\n");
      lichen_deinit();
      return false;
   }
#endif

   RARCH_LOG("[Lichen]: Done initializing!\n");

   return true;
}

lua_State* lichen_get_state(void)
{
   if (!mainThreadState)
      return NULL;

#ifdef HAVE_THREADS
   if (sthread_get_current_thread_id() == mainThreadId)
      return mainThreadState;
   else
      return videoThreadState;
#endif

   return mainThreadState;
}

void lichen_deinit(void)
{
   if (mainThreadState)
      lua_close(mainThreadState);

#ifdef HAVE_THREADS
   lichen_free_lock();
#endif
}
