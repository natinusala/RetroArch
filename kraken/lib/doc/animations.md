# Animations module

## Module functions

### `animations.push(subject, target_value, duration, easing, cb)`

Starts an animation on the global number `subject` (you must give the name of the variable). The subject must have a value before starting the animation.

The value of the subject will asynchronously go from its current value to the target value during the given duration.

The easing function is one of the `animations.easing` constants. The callback can be either `nil` or the name of a global function that will be executed when the animation is finished.

### Constants

Please see `animations.lua` for the list of the easing constants the module provides.
