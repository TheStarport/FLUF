# Story Factions Module

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