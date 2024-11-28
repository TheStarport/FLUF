#pragma once
enum class IClientVTable
{
    FireWeapon,
    ActivateEquip,
    ActivateCruise,
    ActivateThruster,
    SetTarget,
    unknown_6,
    EnterTradeLane,
    LeaveTradeLane,
    JettisonCargo,
    SendPacket,
    Nullsub,
    Login,
    CharacterInfo,
    CharacterSelect,
    Shutdown,
    DispatchMsgs,
    CdpProxyDisconnect,
    CdpProxyGetSendQSize,
    CdpProxyGetSendQBytes,
    CdpProxyGetLinkSaturation,
    SetShipArch,
    SetHullStatus,
    SetCollisionGroups,
    SetEquipment,
    unknown_26,
    AddItemToCharacter,
    unknown_28,
    SetStartRoom,
    DestroyCharacter,
    UpdateCharacter,
    CompleteCharacterList,
    ScriptBehaviour,
    DestroyScriptBehaviour,
    CompleteScriptBehaviourList,
    unknown_36,
    unknown_37,
    CompleteAmbientScriptList,
    DestroyMissionComputer,
    UpdateMissionComputer,
    CompleteMissionComputerList,
    MissionVendorAcceptance,
    MissionVendorEmptyReason,
    unknown_44,
    UpdateNewsBroadcast,
    CompleteNewsBroadcastList,
    CreateSolar,
    CreateShip,
    CreateLoot,
    CreateMine,
    CreateGuided,
    CreateCounter,
    unknown_53,
    unknown_54,
    UpdateObject,
    DestroyObject,
    ActivateObject,
    SystemSwitchOut,
    JumpInComplete,
    Land,
    Launch,
    RequestShip,
    unknown_63,
    DamageObject,
    ItemTractored,
    UseItem,
    SetReputation,
    unknown_68,
    SendComm,
    unknown_70,
    SetMissionMessage,
    unknown_72,
    SetMissionObjectives,
    unknown_74,
    unknown_75,
    MarkObject,
    unknown_77,
    SetCash,
    unknown_79,
    unknown_80,
    unknown_81,
    unknown_82,
    unknown_83,
    RequestReturned,
    unknown_85,
    unknown_86,
    ObjectCargoUpdate,
    BeginFuse,
    unknown_89,
    unknown_90,
    unknown_91,
    SetWeaponGroup,
    SetVisitState,
    RequestBestPath,
    RequestPlayerStats,
    unknown_96,
    RequestGroupPositions,
    SetMissionLog,
    SetInterfaceState,
    unknown_100,
    unknown_101,
    PlayerInitiateTrade,
    PlayerTradeTarget,
    PlayerAcceptTrade,
    PlayerSetTradeMoney,
    PlayerAddTradeEquip,
    PlayerRemoveTradeEquip,
    PlayerRequestTrade,
    PlayerStopTradeRequest,
    ScanNotify,
    PlayerListUpdate,
    PlayerIsLeavingServer,
    PlayerListUpdate2,
    MiscObjectUpdate6,
    MiscObjectUpdate7,
    MiscObjectUpdate1,
    MiscObjectUpdate2,
    MiscObjectUpdate3,
    MiscObjectUpdate4,
    MiscObjectUpdate5,
    unknown_121,
    FormationUpdate,
    unknown_123,
    unknown_124,
    unknown_125,
    unknown_126,
    RemoteStart = 0x3B2A0,
    RemoteEnd = 0x3B494,
    LocalStart = 0x33250,
    LocalEnd = 0x33444
};

enum class IServerVTable
{
    FireWeapon,
    ActivateEquip,
    ActivateCruise,
    ActivateThruster,
    SetTarget,
    TractorObjects,
    EnterTradeLane,
    LeaveTradeLane,
    JettisonCargo,
    Startup,
    Shutdown,
    Update,
    ElapseTime,
    Nullopt1,
    SwapConnections,
    Nullopt2,
    Disconnect,
    Connect,
    Login,
    CharacterInfoRequest,
    CharacterSelect,
    Nullopt3,
    CreateNewCharacter,
    DestroyCharacter,
    CharacterSkipAutosave,
    ReqShipArch,
    ReqHullStatus,
    ReqCollisionGroups,
    ReqEquipment,
    ReqCargo,
    RequestAddItem,
    RequestRemoveItem,
    RequestModifyItem,
    RequestSetCash,
    RequestChangeCash,
    BaseEnter,
    BaseExit,
    LocationEnter,
    LocationExit,
    BaseInfoRequest,
    LocationInfoRequest,
    ObjectSelect,
    GoodVaporized,
    MissionResponse,
    TradeResponse,
    GoodBuy,
    GoodSell,
    SystemSwitchOutComplete,
    PlayerLaunch,
    LaunchComplete,
    JumpInComplete,
    Hail,
    ObjectUpdate,
    MunitionCollision,
    SPBadLandsObjCollision,
    ObjectCollision,
    SPRequestUseItem,
    SPRequestInvincibility,
    SaveGame,
    MissionSaveB,
    RequestEvent,
    RequestCancel,
    MineAsteroid,
    CommComplete,
    RequestCreateShip,
    SPScanCargo,
    SetManeuver,
    InterfaceItemUsed,
    AbortMission,
    RTCDone,
    SetWeaponGroup,
    SetVisitedState,
    RequestBestPath,
    RequestPlayerStats,
    PopUpDialog,
    RequestGroupPositions,
    SetMissionLog,
    SetInterfaceState,
    RequestRankLevel,
    InitiateTrade,
    TerminateTrade,
    AcceptTrade,
    SetTradeMoney,
    AddTradeEquip,
    DelTradeEquip,
    RequestTrade,
    StopTradeRequest,
    RequestDifficultyScale,
    GetServerID,
    GetServerSig,
    DumpPacketStats,
    Dock,
    PushToServer,
    RemoteStart = 0x3B4A0,
    RemoteEnd = 0x3B610,
    LocalStart = 0x33450,
    LocalEnd = 0x335C0
};
