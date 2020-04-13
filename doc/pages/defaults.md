Defaults
========

VUEngine includes defaults for common Virtual Boy software features.


Splash screens
--------------

Default versions of common screens are available in the `VUEngine Barebone` project. Those can easily be directly used, modified, replaced and/or reordered and eliminate the need to create those by hand.

- Precaution screen
- Adjustment screen
- Automatic Pause selection screen
- Language selection screen
- Automatic Pause screen


Low Battery indicator
---------------------

When the Virtual Boy's batteries are low, a flashing low battery indicator is shown on screen. To enable this feature, define the `__LOW_BATTERY_INDICATOR` macro in your game's `config.h` file.

The indicator's default position is {45, 1}. It can be changed through the `__LOW_BATTERY_INDICATOR_POS_X` and `__LOW_BATTERY_INDICATOR_POS_Y` macros. You can furthermore change the initial delay of the low battery indicator through the `__LOW_BATTERY_INDICATOR_INITIAL_DELAY` macro as well as the blinking delay through the `__LOW_BATTERY_INDICATOR_BLINK_DELAY` macro in your game's `config.h` file.


Automatic Pause
---------------

An Automatic Pause screen is shown after approximately 30 minutes of gameplay.

The amount of time after which the Automatic Pause screen is shown can be changed through the `__AUTO_PAUSE_DELAY` setting, to be made in your projects's `config.h` file.

You can also set your own state to be used for the Automatic Pause screen using the `Game::setAutomaticPauseState` function. Here's an example:

	Game::setAutomaticPauseState(Game::getInstance(), (GameState)CustomAutoPauseScreenState::getInstance());
