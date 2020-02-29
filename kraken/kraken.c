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

#include "kraken.h"

#include <stdlib.h>

#include "../verbosity.h"

#include "kraken_lua_user.h"

#include "kraken_lib.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#ifdef HAVE_THREADS
#include <rthreads/rthreads.h>

static uintptr_t mainThreadId;
static lua_State *videoThreadState;
#endif

static lua_State *mainThreadState;

bool kraken_init(void)
{
   /* TODO: don't do anything if there are no addons */

   /* Safety check */
   if (mainThreadState)
   {
      RARCH_LOG("[Kraken]: Already initialized\n");
      return true;
   }

   /* Create state */
#ifdef HAVE_THREADS
   kraken_init_lock();
#endif
   mainThreadState = luaL_newstate();

   if (!mainThreadState)
   {
      RARCH_ERR("[Kraken]: Failed to initialize main thread state\n");
      return false;
   }

   /* Load standard libs */
   luaL_openlibs(mainThreadState);

   /* Load Kraken library */
   kraken_load_lib(mainThreadState);

   /* Load and execute addons */
   /* TODO: properly do it */
   if (luaL_dofile(mainThreadState, "test.lua"))
      RARCH_ERR("[Kraken]: Unable to load addon test.lua: %s\n", kraken_get_error(mainThreadState));

   /* Spawn video thread if needed */
   /* TODO: Is the video thread garbage collected
      if nothing runs on it for a while? */
   /* TODO: make a table of thread id -> Lua state */
#ifdef HAVE_THREADS
   mainThreadId      = sthread_get_current_thread_id();
   videoThreadState  = lua_newthread(mainThreadState);

   if (!videoThreadState)
   {
      RARCH_ERR("[Kraken]: Failed to initialize video thread state\n");
      kraken_deinit();
      return false;
   }
#endif

   RARCH_LOG("[Kraken]: Done initializing\n");

   return true;
}

const char* kraken_get_error(lua_State* state)
{
   const char* error = lua_tostring(state, -1);

   if (error == NULL)
      error = "unknown error";

   return error;
}

lua_State* kraken_get_state(void)
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

void kraken_deinit(void)
{
   if (mainThreadState)
      lua_close(mainThreadState);

#ifdef HAVE_THREADS
   kraken_free_lock();
#endif
}
