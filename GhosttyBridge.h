#pragma once
#include "ghostty/ghostty.h"
#include <Windows.h>
#include <GL/gl.h>
#include <cstdint>
#include <vector>
#include <memory>

// Bridge between libghostty and Win32
// Equivalent to the Swift AppDelegate on macOS

// Per-surface state. One instance per terminal tab/window.
// Stored in HWND GWLP_USERDATA and in ghostty_surface userdata for fast lookup.
struct TerminalSession {
    ghostty_surface_t surface = nullptr;
    HWND hwnd = nullptr;

    // OpenGL renderer state (unused in DirectX mode)
    HDC hdc = nullptr;
    HGLRC hglrc = nullptr;

    // Window state for fullscreen/decorations toggle
    bool fullscreen = false;
    bool decorations = true;
    RECT savedRect = {};
    DWORD savedStyle = 0;

    // Size limits from ghostty config
    uint32_t minWidth = 0, minHeight = 0;
    uint32_t maxWidth = 0, maxHeight = 0;
};

class GhosttyBridge {
public:
    static GhosttyBridge& instance() {
        static GhosttyBridge s;
        return s;
    }

    bool initialize();
    void shutdown();

    ghostty_app_t app() const { return m_app; }
    ghostty_config_t config() const { return m_config; }
    bool isInitialized() const { return m_initialized; }

    // Creates a new TerminalSession with its own child window and ghostty surface.
    // If parentHwnd is null, creates a top-level window.
    // Returned pointer is owned by GhosttyBridge.
    TerminalSession* createSurface(HWND parentHwnd);
    void destroySession(TerminalSession* session);

    const std::vector<std::unique_ptr<TerminalSession>>& sessions() const { return m_sessions; }

private:
    GhosttyBridge() = default;
    ~GhosttyBridge() { shutdown(); }
    GhosttyBridge(const GhosttyBridge&) = delete;
    GhosttyBridge& operator=(const GhosttyBridge&) = delete;

    // Runtime callbacks (libghostty -> Win32)
    static void onWakeup(void* userdata);
    static bool onAction(ghostty_app_t app, ghostty_target_s target, ghostty_action_s action);
    static bool onReadClipboard(void* userdata, ghostty_clipboard_e clipboard, void* state);
    static void onConfirmReadClipboard(void* userdata, const char* content, void* state, ghostty_clipboard_request_e req);
    static void onWriteClipboard(void* userdata, ghostty_clipboard_e clipboard, const ghostty_clipboard_content_s* content, size_t count, bool confirm);
    static void onCloseSurface(void* userdata, bool process_exited);

    // Win32 child window for OpenGL/DirectX rendering
    static LRESULT CALLBACK glWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    HWND createGLWindow(HWND parent, TerminalSession* session);

    // OpenGL context for WGL (per-session)
    static bool initOpenGL(TerminalSession* session);
    static void shutdownOpenGL(TerminalSession* session);

    // Look up the session associated with a ghostty_surface_t (via surface userdata).
    static TerminalSession* sessionFromSurface(ghostty_surface_t surface);

    ghostty_app_t m_app = nullptr;
    ghostty_config_t m_config = nullptr;
    bool m_initialized = false;

    std::vector<std::unique_ptr<TerminalSession>> m_sessions;
};
