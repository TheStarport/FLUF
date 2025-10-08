#pragma once
#include "ImportFluf.hpp"

class Fluf;

enum class KeyState
{
    Pressed,
    Released
};

enum class Key
{
    USER_NONE,
    USER_KEY,
    USER_CHAR,
    USER_BUTTON_OVERRIDE, // 00614DE4
    USER_WHEEL,
    USER_MOVE,
    USER_BUTTON0,
    USER_BUTTON1,
    USER_BUTTON2,
    USER_BUTTON3,
    USER_BUTTON4,
    USER_BUTTON5,
    USER_BUTTON6,
    USER_BUTTON7,
    USER_EVENTS_OVERRIDE, // 00614E10
    USER_CANCEL,
    USER_STACKABLE_CANCEL_OVERRIDE, // 00614E18
    USER_ENTER,
    USER_NO_OVERRIDE,  // 00614E20
    USER_YES_OVERRIDE, // 00614E24
    USER_PAUSE,
    USER_RESET,
    USER_SAVE_GAME_OVERRIDE, // 00614E30
    USER_FULLSCREEN,
    USER_USE_ITEM_OVERRIDE,      // 00614E38
    USER_ACTIVATE_ITEM_OVERRIDE, // 00614E3C
    USER_NEXT_ITEM_OVERRIDE,     // 00614E40
    USER_PREV_ITEM_OVERRIDE,     // 00614E44
    USER_SET_TARGET_OVERRIDE,    // 00614E48
    USER_NEXT_TARGET,
    USER_PREV_TARGET,
    USER_NEXT_WEAPON_SUBTARGET_OVERRIDE, // 00614E54
    USER_PREV_WEAPON_SUBTARGET_OVERRIDE, // 00614E58
    USER_NEXT_SUBTARGET,
    USER_PREV_SUBTARGET,
    USER_CLEAR_TARGET,
    USER_NEXT_ENEMY,
    USER_PREV_ENEMY,
    USER_CLOSEST_ENEMY,
    USER_ZOOM_IN_OVERRIDE,     // 00614E70
    USER_ZOOM_OUT_OVERRIDE,    // 00614E74
    USER_ZOOM_TOGGLE_OVERRIDE, // 00614E78
    USER_X_ROTATE,
    USER_X_UNROTATE,
    USER_Y_ROTATE,
    USER_Y_UNROTATE,
    USER_Z_ROTATE,
    USER_Z_UNROTATE,
    USER_VIEW_RESET_OVERRIDE, // 00614E98
    USER_COCKPIT_CAMERA_MODE,
    USER_LOOK_ROTATE_CAMERA_LEFT,
    USER_LOOK_ROTATE_CAMERA_RIGHT,
    USER_LOOK_ROTATE_CAMERA_UP,
    USER_LOOK_ROTATE_CAMERA_DOWN,
    USER_LOOK_ROTATE_CAMERA_RESET,
    USER_CHASE_CAMERA_MODE_OVERRIDE, // 00614EB4
    USER_REARVIEW_CAMERA_MODE,
    USER_REARVIEW_CAMERA_MODE_OFF,
    USER_AFTERBURN,
    USER_AFTERBURN_OFF,
    USER_AFTERBURN_ON,
    USER_TOGGLE_AUTO_AVOIDANCE,
    USER_TOGGLE_AUTO_LEVEL,
    USER_TOGGLE_LEVEL_CAMERA,
    USER_WARP_OVERRIDE,          // 00614ED8
    USER_ENGINE_TOGGLE_OVERRIDE, // 00614EDC
    USER_SET_THROTTLE_OVERRIDE,  // 00614EE0
    USER_INC_THROTTLE,
    USER_DEC_THROTTLE,
    USER_CRUISE,
    USER_THROTTLE_0_OVERRIDE,   // 00614EF0
    USER_THROTTLE_10_OVERRIDE,  // 00614EF4
    USER_THROTTLE_20_OVERRIDE,  // 00614EF8
    USER_THROTTLE_25_OVERRIDE,  // 00614EFC
    USER_THROTTLE_30_OVERRIDE,  // 00614F00
    USER_THROTTLE_40_OVERRIDE,  // 00614F04
    USER_THROTTLE_50_OVERRIDE,  // 00614F08
    USER_THROTTLE_60_OVERRIDE,  // 00614F0C
    USER_THROTTLE_70_OVERRIDE,  // 00614F10
    USER_THROTTLE_75_OVERRIDE,  // 00614F14
    USER_THROTTLE_80_OVERRIDE,  // 00614F18
    USER_THROTTLE_90_OVERRIDE,  // 00614F1C
    USER_THROTTLE_100_OVERRIDE, // 00614F20
    USER_RADAR_OVERRIDE,        // 00614F24
    USER_CONTACT_LIST,
    USER_RADAR_ZOOM,
    USER_SWITCH_TO_PLAYER_SHIP,
    USER_SWITCH_TO_WEAPON_LIST,
    USER_SWITCH_TO_TARGET,
    USER_FORMATION_LIST,
    USER_RADAR_ZOOM_IN_OVERRIDE,  // 00614F40
    USER_RADAR_ZOOM_OUT_OVERRIDE, // 00614F44
    USER_RADIO,
    USER_CHAT,
    USER_STATUS,
    USER_TARGET,
    USER_STATUS_MODE_OVERRIDE,      // 00614F58
    USER_TOGGLE_MAINFRAME_OVERRIDE, // 00614F5C
    USER_NN_OVERRIDE,               // 00614F60
    USER_NAV_MAP,
    USER_NAVMAP_OVERRIDE, // 00614F68
    USER_PLAYER_STATS,
    USER_INVENTORY,
    USER_STORY_STAR,
    USER_CHAT_WINDOW,
    USER_GROUP_WINDOW,
    USER_HELP,
    USER_INVENTORY_CLOSE,
    NN_TOGGLE_OPEN_OVERRIDE, // 00614F88
    USER_MINIMIZE_HUD,
    USER_DISPLAY_LAST_OBJECTIVE,
    USER_COLLECT_LOOT,
    USER_ACTIONS_OVERRIDE,                // 00614F98
    USER_MANEUVER_OVERRIDE,               // 00614F9C
    USER_SET_MANEUVER_DIRECTION_OVERRIDE, // 00614FA0
    USER_MANEUVER_WINDOW_OVERRIDE,        // 00614FA4
    USER_MANEUVER_TRAIL,
    USER_MANEUVER_AFTERBURNER_OVERRIDE, // 00614FAC
    USER_MANEUVER_EVADE,
    USER_MANEUVER_ENGINEKILL,
    USER_MANEUVER_BRAKE_REVERSE,
    USER_MANEUVER_DOCK,
    USER_MANEUVER_GOTO,
    USER_MANEUVER_FACE,
    USER_MANEUVER_CRUISE_OVERRIDE,     // 00614FC8
    USER_MANEUVER_TRADE_LANE_OVERRIDE, // 00614FC
    USER_MANEUVER_DRASTIC_EVADE,
    USER_MANEUVER_FORMATION,
    USER_MANEUVER_STRAFE,
    USER_MANEUVER_TRAIL_CLOSER,
    USER_MANEUVER_TRAIL_FARTHER,
    USER_MANEUVER_CORKSCREW_EVADE,
    USER_MANEUVER_SLIDE_EVADE,
    USER_MANEUVER_SLIDE_EVADE_LEFT,
    USER_MANEUVER_SLIDE_EVADE_RIGHT,
    USER_MANEUVER_SLIDE_EVADE_UP,
    USER_MANEUVER_SLIDE_EVADE_DOWN,
    USER_ACTIVATE_MANEUVER_OVERRIDE, // 00614FFC
    USER_SCAN_CARGO,
    USER_TRACTOR_BEAM,
    USER_JAMMER_OVERRIDE,  // 00615008
    USER_STEALTH_OVERRIDE, // 0061500C
    USER_CLOAK_OVERRIDE,   // 00615010
    USER_REPAIR_HEALTH,
    USER_REPAIR_SHIELD,
    USER_WEAPON_GROUP1,
    USER_WEAPON_GROUP2,
    USER_WEAPON_GROUP3,
    USER_WEAPON_GROUP4,
    USER_WEAPON_GROUP5,
    USER_WEAPON_GROUP6,
    USER_TOGGLE_WEAPON1,
    USER_TOGGLE_WEAPON2,
    USER_TOGGLE_WEAPON3,
    USER_TOGGLE_WEAPON4,
    USER_TOGGLE_WEAPON5,
    USER_TOGGLE_WEAPON6,
    USER_TOGGLE_WEAPON7,
    USER_TOGGLE_WEAPON8,
    USER_TOGGLE_WEAPON9,
    USER_TOGGLE_WEAPON10,
    USER_FIRE_WEAPON1,
    USER_FIRE_WEAPON2,
    USER_FIRE_WEAPON3,
    USER_FIRE_WEAPON4,
    USER_FIRE_WEAPON5,
    USER_FIRE_WEAPON6,
    USER_FIRE_WEAPON7,
    USER_FIRE_WEAPON8,
    USER_FIRE_WEAPON9,
    USER_FIRE_WEAPON10,
    USER_FIRE_WEAPON_GROUP1,
    USER_FIRE_WEAPON_GROUP2,
    USER_FIRE_WEAPON_GROUP3,
    USER_FIRE_WEAPON_GROUP4,
    USER_FIRE_WEAPON_GROUP5,
    USER_FIRE_WEAPON_GROUP6,
    USER_ASSIGN_WEAPON_GROUP1,
    USER_ASSIGN_WEAPON_GROUP2,
    USER_ASSIGN_WEAPON_GROUP3,
    USER_ASSIGN_WEAPON_GROUP4,
    USER_ASSIGN_WEAPON_GROUP5,
    USER_ASSIGN_WEAPON_GROUP6,
    USER_REMAPPABLE_LEFT,
    USER_REMAPPABLE_RIGHT,
    USER_REMAPPABLE_UP,
    USER_REMAPPABLE_DOWN,
    USER_FIRE_FORWARD,
    USER_LAUNCH_MISSILES,
    USER_LAUNCH_MINES,
    USER_LAUNCH_COUNTERMEASURES,
    USER_AUTO_TURRET,
    USER_LAUNCH_TORPEDOS,
    USER_LAUNCH_CRUISE_DISRUPTORS,
    USER_TURN, // 006150E0
    USER_NEXT_OBJECT,
    USER_PREV_OBJECT,
    USER_EXIT_GAME,
    USER_MANEUVER_FREE_FLIGHT,
    USER_FIRE_WEAPONS,
    USER_TURN_SHIP,
    USER_GROUP_INVITE,
    USER_TRADE_REQUEST,
    USER_SCREEN_SHOT
};

struct KeyMapping
{
        enum class KeyMod : uint8_t
        {
            None = 0,
            Shift = 1,
            Control = 4,
            Alt = 16
        };

        std::string name;
        KeyMod mod;
        int virtualKey;
};

class FlufModule;
using KeyFunc = bool (FlufModule::*)(KeyState state);
class KeyManager
{
        struct OverriddenKey
        {
                std::string moduleName;
                KeyFunc function;
        };

        inline static KeyManager* instance;
        std::unordered_map<Key, OverriddenKey> overriddenKeys;
        const std::unordered_set<Key> overrideableKeys{
            Key::USER_NN_OVERRIDE,
            Key::USER_NO_OVERRIDE,
            Key::USER_YES_OVERRIDE,
            Key::USER_WARP_OVERRIDE,
            Key::USER_CLOAK_OVERRIDE,
            Key::USER_RADAR_OVERRIDE,
            Key::USER_BUTTON_OVERRIDE,
            Key::USER_EVENTS_OVERRIDE,
            Key::USER_JAMMER_OVERRIDE,
            Key::USER_NAVMAP_OVERRIDE,
            Key::USER_ZOOM_IN_OVERRIDE,
            Key::USER_ACTIONS_OVERRIDE,
            Key::USER_STEALTH_OVERRIDE,
            Key::USER_USE_ITEM_OVERRIDE,
            Key::NN_TOGGLE_OPEN_OVERRIDE,
            Key::USER_MANEUVER_OVERRIDE,
            Key::USER_NEXT_ITEM_OVERRIDE,
            Key::USER_PREV_ITEM_OVERRIDE,
            Key::USER_SAVE_GAME_OVERRIDE,
            Key::USER_THROTTLE_0_OVERRIDE,
            Key::USER_THROTTLE_10_OVERRIDE,
            Key::USER_THROTTLE_20_OVERRIDE,
            Key::USER_THROTTLE_30_OVERRIDE,
            Key::USER_THROTTLE_40_OVERRIDE,
            Key::USER_THROTTLE_50_OVERRIDE,
            Key::USER_THROTTLE_60_OVERRIDE,
            Key::USER_THROTTLE_70_OVERRIDE,
            Key::USER_THROTTLE_80_OVERRIDE,
            Key::USER_THROTTLE_90_OVERRIDE,
            Key::USER_THROTTLE_100_OVERRIDE,
            Key::USER_SET_TARGET_OVERRIDE,
            Key::USER_STATUS_MODE_OVERRIDE,
            Key::USER_VIEW_RESET_OVERRIDE,
            Key::USER_ZOOM_TOGGLE_OVERRIDE,
            Key::USER_SET_THROTTLE_OVERRIDE,
            Key::USER_RADAR_ZOOM_IN_OVERRIDE,
            Key::USER_ACTIVATE_ITEM_OVERRIDE,
            Key::USER_ENGINE_TOGGLE_OVERRIDE,
            Key::USER_SET_MANEUVER_DIRECTION_OVERRIDE,
            Key::USER_PREV_WEAPON_SUBTARGET_OVERRIDE,
            Key::USER_NEXT_WEAPON_SUBTARGET_OVERRIDE,
            Key::USER_MANEUVER_AFTERBURNER_OVERRIDE,
            Key::USER_MANEUVER_TRADE_LANE_OVERRIDE,
            Key::USER_ACTIVATE_MANEUVER_OVERRIDE,
            Key::USER_TOGGLE_MAINFRAME_OVERRIDE,
            Key::USER_STACKABLE_CANCEL_OVERRIDE,
            Key::USER_CHASE_CAMERA_MODE_OVERRIDE,
            Key::USER_MANEUVER_WINDOW_OVERRIDE,
            Key::USER_MANEUVER_CRUISE_OVERRIDE,
            Key::USER_RADAR_ZOOM_OUT_OVERRIDE,
        };

        bool HandleKey(Key key);
        static void HandleKeyNaked();
        void GenerateKeyMap();

        std::vector<KeyMapping> userKeyMap;

    public:
        explicit KeyManager();
        ~KeyManager();
        [[nodiscard]]
        const std::vector<KeyMapping>& GetKeyMap() const;

        /**
         * @brief This function allows you to set up a callback for when certain keys are pressed and conditionally decide to process said key commands.
         * If a key has already been registered by another FLUF module, a critical error with a message box will be raised. There are 49 unused key commands
         * in the base game, and theoretically more can be added if this limit is ever reached.
         * @param module The module requesting a new key to be registered
         * @param newName The new name of the key command for the purpose of INI binding. It's suggested that you prefix the command with FLUF
         * e.g. FLUF_OPEN_UI
         * @param key The existing key command you are wanting to override. Commands that do nothing in vanilla are suffixed with _OVERRIDE.
         * @param function The function that you want to be called when the desired key is pressed. The return value indicates whether
         * the key press should be passed into the base game's command handler. This is useful for overriding or preventing existing keyboard commands in
         * certain circumstances. Returning true indicates that you have handled the command, returning false will pass the key to the base game.
         * @param suppressWarnings If true no warning will be logged if you are overriding a key that is not suffixed with _OVERRIDE.
         */
        FLUF_API void RegisterKey(FlufModule* module, std::string_view newName, Key key, KeyFunc function, bool suppressWarnings = false);
        /**
         * @brief Deactivate a callback from further execution, this will not restore vanilla functionality in the event of that being override,
         * it simply suppresses the function call.
         * @param key The key to prevent the callback being called with
         * @return true if the command was successfully removed, false if it was not found to be registered.
         */
        FLUF_API bool UnregisterKey(Key key);
};
