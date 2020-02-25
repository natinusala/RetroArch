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

#include "gfx_widgets.h"

#include <duktape.h>

#include <verbosity.h>q

static duk_ret_t js_native_rarch_log(duk_context* ctx)
{
   RARCH_LOG("%s", duk_to_string(ctx, 0));
   return 0;
}

void gfx_widgets_js_register_functions(duk_context* ctx)
{
   duk_push_c_function(ctx, js_native_rarch_log, 1);
   duk_put_global_string(ctx, "RARCH_LOG");
}
