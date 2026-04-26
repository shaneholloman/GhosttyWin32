#pragma once

#include "MainWindow.g.h"
#include "ghostty.h"
#include "GhosttyApp.h"
#include "ImeBuffer.h"
#include "Tab.h"
#include "Tabs.h"

namespace winrt::GhosttyWin32::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();
        ~MainWindow();

        // Best-effort cleanup invoked from SetUnhandledExceptionFilter.
        // Walks live tabs and closes their composition surface handles so
        // DComp drops its driver-side references before the OS kills the
        // process — reduces the chance the next launch inherits corrupted
        // NVIDIA state.
        static long __stdcall OnUnhandledException(struct _EXCEPTION_POINTERS* info) noexcept;

    private:
        void InitGhostty();
        void CreateTab();
        Tab* ActiveTab();

        std::unique_ptr<GhosttyApp> m_ghostty;
        HWND m_hwnd = nullptr;
        winrt::Windows::UI::Text::Core::CoreTextEditContext m_editContext{ nullptr };
        ImeBuffer m_ime;
        Tabs m_tabs;
    };
}

namespace winrt::GhosttyWin32::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
