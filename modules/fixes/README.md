# FLUF Fixes Module

Fixes applies a number of quality of life and bug fixes to the game:

- Allows for 'negative' damage, i.e. a value of `-10` will increase target hull by `10` rather than being interpreted as
  `INT_MAX`.
- Prevents NPCs from attempting to tractor in loot they cannot fit in their cargo hold.
- Patches the message the `ERROR:General:StreamingSound: missing MP3 codec` from `FLSpew.txt`.
    - Windows: The patch points the game at the correct codec, resolving the error.
    - Linux: The message is patched out altogether.
- Allows dynamic asteroids to respect given properties, i.e. mass, `hit_points`. `Collision` damage cannot be changed.
- Enables the `damage_per_fire` property on weapons, allowing the creation of weapons that take damage when they are
  fired.
- Correctly displays political zones in multiplayer, enables level and reputation requirements for multiplayer.
- Ensures that shields are taken offline after they have been destroyed.
- Lifts the limit on the number of possible ship classes. The configuration for these can be found in
  `EXE/modules/config/ship_classes.yml`. This file is generated on first run.
- Enables wheel scrolling in a variety of different windows.
- Changes the dash arrow to an en-dash and arrow, but only if it hasn't been patched by something else.
- Place the sectors letters before numbers.
- Allow viewing of server info for incompatible versions.
- Forces the primary weapon group upon launch.
- Fix a number of sounds not being played (Multiple HpFire points, reverse thrusters).
- Allow undocking from non-targetable objects. Game will still crash if no object exists.