#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "ghostty.h"

// Helper for building ghostty input modifier flags from Win32 key state.
struct InputHelper {
    static ghostty_input_mods_e currentMods() {
        int mods = 0;
        if (GetKeyState(VK_SHIFT) & 0x8000)   mods |= GHOSTTY_MODS_SHIFT;
        if (GetKeyState(VK_CONTROL) & 0x8000)  mods |= GHOSTTY_MODS_CTRL;
        if (GetKeyState(VK_MENU) & 0x8000)     mods |= GHOSTTY_MODS_ALT;
        return static_cast<ghostty_input_mods_e>(mods);
    }
};
