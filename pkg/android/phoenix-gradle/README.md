Phoenix Gradle Build
====================

Implements a Gradle build based on the existing phoenix sources.

It is currently only useful for running and debugging the RetroArch frontend in Android Studio.
This is caused by the fact that this build can't support the same older API level that the old Ant 
based build does. The minimum supported API level for this build is 16. Also this will not build the 
mips variant cause support for this architecture has long been removed from the Android NDK.
The only file that had to be duplicated is the AndroidManifest.xml because the modern Android build
won't allow SDK versions defined in this file anymore. It's also easier to change the app name this way.

To get this running follow these steps:

* Install the Android SDK Platform 28
* Install the latest Android NDK
* Import the project into Android Studio
* Make sure to select the appropriate build variant for your device (32 or 64 bit)

Sideloading a core
------------------

The `CoreSideloadActivity` activity allows you to sideload and run a core (with content) from your computer through ADB.

**Don't forget to kill the task on your device before running the commands again, it will not reload the core otherwise!**

Usage :

```
adb push <core> /data/local/tmp
adb shell am start -n <package>/com.retroarch.browser.debug.CoreSideloadActivity --es "LIBRETRO" "/data/local/tmp/<core>" --es "ROM" "<content>"
```

Where `<package>` is the target RetroArch app package name :
  - `com.retroarch` (RetroArch)
  - `com.retroarch.aarch64` (RetroArch64)
`<content>` is the path to the content to load (on your device) (optional)
and `<core>` is the path to the core to sideload (on your computer).
