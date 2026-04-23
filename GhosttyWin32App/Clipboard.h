#pragma once

#include <string>
#include <cstdint>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

// Pure clipboard logic separated from UI for testability.
// The read/write methods handle Win32 clipboard API + UTF-8/UTF-16 conversion.
class Clipboard {
public:
    // Read UTF-8 text from the system clipboard.
    // Returns empty string on failure.
    static std::string readText(HWND hwnd) {
        if (!OpenClipboard(hwnd)) return {};
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (!hData) { CloseClipboard(); return {}; }
        wchar_t* wstr = static_cast<wchar_t*>(GlobalLock(hData));
        if (!wstr) { CloseClipboard(); return {}; }

        std::string result;
        int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
        if (len > 1) {
            result.resize(len - 1);
            WideCharToMultiByte(CP_UTF8, 0, wstr, -1, result.data(), len, nullptr, nullptr);
        }
        GlobalUnlock(hData);
        CloseClipboard();
        return result;
    }

    // Write UTF-8 text to the system clipboard.
    // Returns true on success.
    static bool writeText(HWND hwnd, const char* utf8, int utf8Len = -1) {
        int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8, utf8Len, nullptr, 0);
        if (wlen <= 0) return false;
        if (!OpenClipboard(hwnd)) return false;
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, wlen * sizeof(wchar_t));
        if (!hMem) { CloseClipboard(); return false; }
        wchar_t* dest = static_cast<wchar_t*>(GlobalLock(hMem));
        MultiByteToWideChar(CP_UTF8, 0, utf8, utf8Len, dest, wlen);
        if (utf8Len != -1) dest[wlen - 1] = L'\0';  // null-terminate for non-null-terminated input
        GlobalUnlock(hMem);
        SetClipboardData(CF_UNICODETEXT, hMem);
        CloseClipboard();
        return true;
    }

    // Write UTF-8 text from a ghostty selection (known length, not null-terminated).
    static bool writeSelection(HWND hwnd, const char* text, size_t textLen) {
        int wlen = MultiByteToWideChar(CP_UTF8, 0, text, static_cast<int>(textLen), nullptr, 0);
        if (wlen <= 0) return false;
        if (!OpenClipboard(hwnd)) return false;
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (wlen + 1) * sizeof(wchar_t));
        if (!hMem) { CloseClipboard(); return false; }
        wchar_t* dest = static_cast<wchar_t*>(GlobalLock(hMem));
        MultiByteToWideChar(CP_UTF8, 0, text, static_cast<int>(textLen), dest, wlen);
        dest[wlen] = L'\0';
        GlobalUnlock(hMem);
        SetClipboardData(CF_UNICODETEXT, hMem);
        CloseClipboard();
        return true;
    }
};
