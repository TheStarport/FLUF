			  Story Factions by Jason Hood

				  Version 1.00


Story Factions allows customization of the factions that are excluded from the
reputation list.  Using this DLL defines all such factions, completely replacing
the defaults (provided the ini exists and actually contains factions).	To use
it, copy STORYFACTIONS.DLL to the EXE directory and add it to the [Libraries]
section of EXE\dacom.ini.  Then copy STORYFACTIONS.INI to the DATA directory and
edit it as required.


Jason Hood, 23 March, 2010.
http://freelancer.adoxa.cjb.net/

Fluf adaptation by Josbyte, 2025 

# Story Factions Module

WARNING: This module is not compatible with Freelancer HD mod, as it has StoryFactions already implemented.

This module allows you to define which factions are considered "story factions" in Freelancer. Story factions are excluded from the reputation list in the game.

## Configuration

The module uses a YAML configuration file located at `modules/config/story_factions.yml`. The file has the following format:

```yaml
factions:
  - li_n_grp
  - li_p_grp
  - br_n_grp
  - br_p_grp
  - ku_n_grp
  - ku_p_grp
  - rh_n_grp
  - rh_p_grp
```

You can add or remove faction names as needed.