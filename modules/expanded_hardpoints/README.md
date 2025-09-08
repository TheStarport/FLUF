# Expanded Hardpoints

Expanded hardpoints allows you to apply 'subtypes' to equipment and ship hardpoints to allow you to create effectively infinite equipment classes, including internal equipment such as scanners and armors.
Plugin will block mounting upon purchase as well as manual mounting of equipment considered invalid if a subtype exists but does not match for the ship/currently free hardpoints.
 
## Configuration

In any equipment block, add the subtypes by adding the following line(s) below the "nickname" line:
hp_subtype = type1, type2, type3

In ship block, add a similar line where the subtype is meant to match, then list the hardpoints
hp_subtype = light_fighter_gun, HpWeapon01, HpWeapon02, HpWeapon03