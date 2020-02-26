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
