# Matthew Smith Game Launcher

A simple C++ launcher application for the enhanced Manic Miner and Jet Set Willy games.

## Features

- 🎮 **Easy Game Selection**: Choose between Manic Miner and Jet Set Willy from a simple menu
- ✅ **Game Detection**: Automatically detects if games are built and available
- 📋 **Controls Reference**: Built-in controls and feature documentation
- ℹ️ **Game Information**: About section with history and technical details
- 🖥️ **Clean Interface**: Console-based with clear navigation

## Building

```bash
cd /home/jon/GameLauncher
make
```

## Running

```bash
./game-launcher
```

## Installation

To install the launcher to your local bin directory:

```bash
make install
```

Then you can run it from anywhere:

```bash
game-launcher
```

## Menu Options

1. **[1] Manic Miner** - Launch the original 1983 platform game
2. **[2] Jet Set Willy** - Launch the 1984 sequel with 60 rooms
3. **[3] Show Controls** - View complete controls and features guide
4. **[4] About** - Information about the games and enhancements
5. **[Q] Quit** - Exit the launcher

## Requirements

- The launcher expects the games to be located at:
  - `~/ManicMiner/manicminer`
  - `~/JetSetWilly/jetsetwilly`
- Games must be built using `make` in their respective directories
- C++17 compatible compiler (g++)

## Game Features

Both games include the enhanced features:
- **F11** - Toggle fullscreen/windowed mode
- **TAB/Pause** - Pause/unpause gameplay
- **M** - Mute/unmute audio
- Resizable windows with proper scaling
- Modern SDL2 compatibility
- Bug fixes from original versions

## Troubleshooting

**"Game not found" errors:**
- Ensure games are built: `cd ~/ManicMiner && make` or `cd ~/JetSetWilly && make`
- Check that executables exist and are executable

**Compilation errors:**
- Ensure you have g++ with C++17 support installed
- On some systems you may need: `sudo pacman -S gcc` or equivalent

## Uninstalling

To remove from system:
```bash
make uninstall
make clean
```
