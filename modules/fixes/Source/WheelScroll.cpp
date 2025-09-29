#include "PCH.hpp"

#include "Fixes.hpp"
#include "Utils/MemUtils.hpp"

#define ADDR_CHATCONTROL_WHEEL   0x5CFB0C           // Chat history
#define ADDR_LOADSAVE_WHEEL      0x5D1DA4           // Load or Save Game
#define ADDR_NAVMAP_WHEEL        0x5D2FCC           // waypoints
#define ADDR_KEY_WHEEL           0x5D4AA4           // key list
#define ADDR_TEXT_WHEEL          0x5E47B4           // info, stats, log
#define ADDR_CTRL_WHEEL          0x5E4F6C           // text control
#define ADDR_SERVER_WHEEL        0x5E5164           // servers
#define ADDR_UISCROLLTABLE_WHEEL 0x5E5294           // broadcast & private chat lists
#define ADDR_SCROLL_WHEEL        0x5E55F4           // scroll bar
#define ADDR_DESC_WHEEL          0x5E591C           // server description
#define ADDR_LOAD_WHEEL          0x5E5CFC           // Load Game (from the main menu)
#define ADDR_LIST_WHEEL          0x5E6194           // inventory, jobs, news
#define ADDR_WHEEL               ((PDWORD)0x4d845c) // in space

#define CURRENT_CURSOR           ((LPCSTR)0x616858) // pointer to length & string

struct Scroll
{
        BYTE x1[0x6C];
        BYTE flags;
        BYTE x2[0x3B8 - 0x6D];
        int dir;
        float delay;
};

#define VISIBLE 2

struct Window
{
        PVOID vftable;
        Window* parent;
        Window* win1;
        Window* win2; // next/prev/sibling/something...
        char* type;
        int name_len;
        char name[32];

        bool Wheel(int dir);
};

struct
{
        LPCSTR name;
        DWORD offset;
} windows[] = {
    {    "HUD_ContactList",  0x3D4 },
    {         "HUD_Status",  0x58C },
    {     "NeuroNetNavMap",  0xAE0 },
    { "NN_CommodityByBase",  0x348 },
    {          "NN_Dealer",  0x3D0 },
    {       "NN_Inventory",  0x3D0 },
    {        "NN_LoadSave", 0x1758 },
    {   "NN_MissionVendor",  0x428 },
    {            "NN_News",  0x648 },
    {      "NN_PlayerInfo",  0x394 },
    {     "NN_Preferences",  0x4B8 },
    {          "NN_Repair",  0x46C },
    {         "NN_Scanner",  0x3D0 },
};

#define Text_Scroll   0x3F0
#define Load_Scroll   0xD4
#define Server_Scroll 0xC8
#define Base_Scroll   0xFC4

using ScrollUpdate = void(__fastcall*)(Scroll*);

bool Window::Wheel(const int dir)
{
    for (auto win = this; win != nullptr; win = win->parent)
    {
        int ofs = -1;

        if (win->type != nullptr && strcmp(win->type, "UIScroll") == 0)
        {
            ofs = 0;
        }
        else if (win->type != nullptr && (strcmp(win->type, "RichTextWin") == 0 || strcmp(win->type, "UIRichTextWin") == 0))
        {
            ofs = Text_Scroll;
        }
        else if (strcmp(win->name, "CharSelDesc") == 0 || strcmp(win->name, "CharSelTime") == 0)
        {
            win = win->parent;
            ofs = Load_Scroll;
        }
        else if (strcmp(win->name, "ServerTable") == 0)
        {
            win = win->parent;
            ofs = Server_Scroll;
        }
        else if (strncmp(win->name, "NavMapBase", 10) == 0 || strcmp(win->name, "NavMapSystemTitleText") == 0)
        {
            win = win->parent;
            ofs = Base_Scroll;
        }
        else if (strcmp(win->name, "ChatControl") == 0)
        {
            win = win->win2->win2; // MsgWin, UIScroll
            if (win == nullptr)
            {
                return false;
            }
            ofs = 0;
        }
        else if (win->type != nullptr && strcmp(win->type, "UIScrollTable") == 0)
        {
            win = win->win2; // UIScroll
            if (win == nullptr)
            {
                return false;
            }
            ofs = 0;
        }
        else
        {
            for (auto& window : windows)
            {
                if (strcmp(win->name, window.name) == 0)
                {
                    ofs = window.offset;
                    break;
                }
            }
        }
        if (ofs >= 0)
        {
            Scroll* scroll = (ofs == 0) ? reinterpret_cast<Scroll*>(win) : reinterpret_cast<Scroll**>(win)[ofs / 4];
            if (scroll != nullptr && (scroll->flags & VISIBLE))
            {
                scroll->dir = (dir < 0) ? 1 : 0;
                scroll->delay = -1;
                reinterpret_cast<ScrollUpdate>(0x597560)(scroll);
                scroll->delay = -1;
                reinterpret_cast<ScrollUpdate>(0x597560)(scroll);
                scroll->dir = -1;
                return true;
            }
            if (ofs == 0)
            {
                break;
            }
        }
    }

    return false;
}

DWORD originalWheelFunction;
__declspec(naked) void WheelHook()
{
    __asm {
        mov	eax, ds:[0x616858] // current cursor
        test	eax, eax
        jz	thrust
        cmp	dword ptr [eax], 5 // length of "arrow"
        jne	thrust
        mov	eax, [eax+4]
        or	eax, 0x20202020
        cmp	eax, 'orra' // I'll assume the 'w'
        jne	thrust
        pop	edi // it's the arrow cursor, pass the
        pop	esi //  wheel along
        mov	al, 0
        pop	ebx
        ret	4
      thrust:
        jmp	originalWheelFunction
        align	16
    }
}

// Ship dealer scroll bar hooks

#define ADDR_STATS_BAR      ((PBYTE)0x481205 + 3) // selected
#define ADDR_STATS_WIN      ((PDWORD)(0x48120d + 1))
#define ADDR_SHIPSTATS_WIN  ((PDWORD)(0x4b7d4e + 1)) // dealer
#define ADDR_SHIPVALUES_BAR ((PBYTE)0x4b8ee3)
#define ADDR_SCROLL         ((PDWORD)(0x57b003 + 1))

DWORD ScrollOrg;
DWORD StatsOrg, ShipStatsOrg, retAddr;
DWORD window;

__declspec(naked) void StatsHook()
{
    __asm {
        pop	retAddr
        call	StatsOrg
        mov	window, eax
        jmp	retAddr
    }
}

__declspec(naked) void ShipStatsHook()
{
    __asm {
        pop	retAddr
        call	ShipStatsOrg
        mov	window, eax
        jmp	retAddr
    }
}

// Test if the values window is being scrolled by testing for its name.  Do this
// rather than using the pointer, so there's no need to test for the pointer
// being reused for another window.
__declspec(naked) void ScrollHook()
{
    __asm {
        push	esi
        push	edi
        add	esi, 0x18
        mov	edi, 0x5d1cbc // "InfoCard_Values"
        cmp	dword ptr [esi], 'pihS'
        jne	info
        add	esi, 4
      info:
        mov	ecx, 16/4
        repe	cmpsd
        pop	edi
        pop	esi
        jne	done

        push	eax
        push	edi
        mov	ecx, window
        call	ScrollOrg
      done:
        mov	ecx, esi
        jmp	ScrollOrg
    }
}

void Fixes::EnableMenuScrollingWithMouseWheel()
{
    // Enable scrolling in menus

    MemUtils::Protect(reinterpret_cast<PDWORD>(ADDR_CHATCONTROL_WHEEL), 4, false);
    MemUtils::Protect(reinterpret_cast<PDWORD>(ADDR_LOADSAVE_WHEEL), 4, false);
    MemUtils::Protect(reinterpret_cast<PDWORD>(ADDR_NAVMAP_WHEEL), 4, false);
    MemUtils::Protect(reinterpret_cast<PDWORD>(ADDR_KEY_WHEEL), 4, false);
    MemUtils::Protect(reinterpret_cast<PDWORD>(ADDR_TEXT_WHEEL), 4, false);
    MemUtils::Protect(reinterpret_cast<PDWORD>(ADDR_CTRL_WHEEL), 4, false);
    MemUtils::Protect(reinterpret_cast<PDWORD>(ADDR_SERVER_WHEEL), 4, false);
    MemUtils::Protect(reinterpret_cast<PDWORD>(ADDR_UISCROLLTABLE_WHEEL), 4, false);
    MemUtils::Protect(reinterpret_cast<PDWORD>(ADDR_SCROLL_WHEEL), 4, false);
    MemUtils::Protect(reinterpret_cast<PDWORD>(ADDR_DESC_WHEEL), 4, false);
    MemUtils::Protect(reinterpret_cast<PDWORD>(ADDR_LOAD_WHEEL), 4, false);
    MemUtils::Protect(reinterpret_cast<PDWORD>(ADDR_LIST_WHEEL), 4, false);

    using FWheel = bool (Window::*)(int);

    // clang-format off
    *reinterpret_cast<FWheel*>(ADDR_CHATCONTROL_WHEEL) =
    *reinterpret_cast<FWheel*>(ADDR_LOADSAVE_WHEEL)    =
    *reinterpret_cast<FWheel*>(ADDR_NAVMAP_WHEEL)	   =
    *reinterpret_cast<FWheel*>(ADDR_KEY_WHEEL)	   =
    *reinterpret_cast<FWheel*>(ADDR_TEXT_WHEEL)	   =
    *reinterpret_cast<FWheel*>(ADDR_CTRL_WHEEL)	   =
    *reinterpret_cast<FWheel*>(ADDR_SERVER_WHEEL)	   =
    *reinterpret_cast<FWheel*>(ADDR_UISCROLLTABLE_WHEEL) =
    *reinterpret_cast<FWheel*>(ADDR_SCROLL_WHEEL)	   =
    *reinterpret_cast<FWheel*>(ADDR_DESC_WHEEL)	   =
    *reinterpret_cast<FWheel*>(ADDR_LOAD_WHEEL)	   =
    *reinterpret_cast<FWheel*>(ADDR_LIST_WHEEL)	   = &Window::Wheel;
    // clang-format on

    MemUtils::Protect(ADDR_WHEEL, 4);
    originalWheelFunction = *ADDR_WHEEL;
    *ADDR_WHEEL = reinterpret_cast<DWORD>(WheelHook);

    // Add scroll bar to ship dealer
    MemUtils::Protect(ADDR_STATS_BAR, 1);
    MemUtils::Protect(ADDR_STATS_WIN, 4);
    MemUtils::Protect(ADDR_SHIPSTATS_WIN, 4);
    MemUtils::Protect(ADDR_SHIPVALUES_BAR, 2);
    MemUtils::Protect(ADDR_SCROLL, 4);

    *ADDR_STATS_BAR = 0x10;        // change argument to no scroll bar
    ADDR_SHIPVALUES_BAR[0] = 0xEB; // don't turn off the scroll bar
    ADDR_SHIPVALUES_BAR[1] = 0x06;

    NEWOFS(ADDR_STATS_WIN, StatsHook, StatsOrg);
    NEWOFS(ADDR_SHIPSTATS_WIN, ShipStatsHook, ShipStatsOrg);
    NEWOFS(ADDR_SCROLL, ScrollHook, ScrollOrg);
}
