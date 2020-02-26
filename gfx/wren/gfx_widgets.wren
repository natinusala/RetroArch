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

// Base class for all Widgets
class Widget {
   onInit() {}
   onFree() {}
   onContextReset() {}
   onIterate() {}
   onFrame() {}
}

// Wren <-> RetroArch widgets interface
class WidgetsManager {
   // Called by Wren widgets to register themselves
   static registerWidget(name, widget) {
      if (__widgets == null) {
         __widgets = []
      }

      __widgets.add(widget)
      System.print("[WidgetsManager]: Registering widget %(name)")
   }

   // Called by RetroArch
   static init() {
      for (widget in __widgets) {
         widget.onInit()
      }
   }

   static free() {
      for (widget in __widgets) {
         widget.onFree()
      }
   }

   static contextReset() {
      for (widget in __widgets) {
         widget.onContextReset()
      }
   }

   static iterate() {
      for (widget in __widgets) {
         widget.onIterate()
      }
   }

   static frame() {
      for (widget in __widgets) {
         widget.onFrame()
      }
   }
}
