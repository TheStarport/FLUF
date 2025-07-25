			  Story Factions by Jason Hood

				  Version 1.00


Story Factions allows customization of the factions that are excluded from the
reputation list.  Using this DLL defines all such factions, completely replacing
the defaults.

Original version written by Jason Hood, 23 March, 2010
http://adoxa.altervista.org

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