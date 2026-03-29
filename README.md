# GhosttyWin32

Windows host application for [Ghostty](https://github.com/ghostty-org/ghostty) terminal emulator.

Uses the libghostty C API (DLL) with a Win32 window + OpenGL rendering.

## Architecture

```
GhosttyWin32 (C++/Win32)
  └── GhosttyBridge: libghostty C API wrapper
      ├── Win32 window + WGL OpenGL context
      ├── Keyboard input (WM_CHAR + WM_KEYDOWN → ghostty_surface_text/key)
      ├── Mouse input (click, drag, scroll, selection)
      ├── Clipboard (Ctrl+C copy, Ctrl+V paste, right-click copy)
      ├── Config file loading (%LOCALAPPDATA%\ghostty\config)
      ├── DPI scaling (Per-Monitor DPI aware)
      └── ConPTY → cmd.exe/PowerShell

ghostty.dll (Zig)
  ├── Terminal emulator core
  ├── OpenGL renderer
  ├── Font rendering (Freetype + Harfbuzz)
  └── ConPTY subprocess management
```

## Building

### Prerequisites

- Visual Studio 2022+ with C++ desktop development workload
- Zig 0.15.2+
- Windows SDK

### Build ghostty.dll

This requires a forked version of Ghostty with Windows support patches:

```bash
git clone https://github.com/i999rri/ghostty.git
cd ghostty
git switch windows-port
zig build -Doptimize=ReleaseFast
```

Copy the following files to `GhosttyWin32/ghostty/`:
- `zig-out/lib/ghostty.dll`
- `.zig-cache/o/<hash>/ghostty.lib` (small import library ~60KB)
- `include/ghostty.h` (already included in this repo)

### Build GhosttyWin32

Open `GhosttyWin32.slnx` in Visual Studio, set to **Release | x64**, and build.

### Run

Run without debugger for best performance (Ctrl+F5 in Visual Studio, or run the exe directly).

## Configuration

Create `%LOCALAPPDATA%\ghostty\config` to customize settings:

```
font-size=14
```

See [Ghostty documentation](https://ghostty.org/docs/config) for available options.

## Known Issues

- Window resize causes flickering ([#2](https://github.com/i999rri/GhosttyWin32/issues/2))
- Config `command` setting does not take effect ([#3](https://github.com/i999rri/GhosttyWin32/issues/3))
- Selection is not cleared after copy ([#1](https://github.com/i999rri/GhosttyWin32/issues/1))

## Status

This is an experimental Windows port (PoC). See the [windows-port branch](https://github.com/i999rri/ghostty/tree/windows-port) for Ghostty-side changes.

## AI Disclosure

Claude Code was used to assist with development.
