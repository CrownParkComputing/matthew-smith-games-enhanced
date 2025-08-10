# Matthew Smith Game Launcher - GUI Edition

A beautiful GTK4-based GUI launcher for the enhanced Manic Miner and Jet Set Willy games, featuring box art from your Downloads folder.

## Features

- 🎨 **Beautiful GUI**: Modern GTK4 interface with native Linux look and feel
- 🖼️ **Box Art Display**: Shows game box art from your Downloads folder
- 🎮 **Game Cards**: Rich game information with descriptions and status
- ✅ **Smart Detection**: Automatically detects if games are built and available
- 📋 **Integrated Help**: Controls and about dialogs accessible from the header bar
- 🚀 **Easy Launching**: Click game cards to launch games
- ⚡ **Launch Feedback**: Shows launch progress and completion status

## Box Art Integration

The launcher automatically looks for these images in `/home/jon/Downloads/`:
- **Manic Miner**: `Manic_miner_bugbyte.jpg`
- **Jet Set Willy**: `Jet_Set_Willy_(game_box_art).jpg`

If box art is not found, placeholder graphics are displayed instead.

## Building

First, ensure GTK4 is installed:
```bash
# Check if GTK4 is available
make check-deps

# If not installed on CachyOS/Arch:
sudo pacman -S gtk4
```

Build the launcher:
```bash
cd /home/jon/GameLauncherGUI
make
```

## Running

```bash
# From the build directory
./game-launcher-gui

# Or install globally first
make install
~/.local/bin/game-launcher-gui
```

## Interface Overview

### Main Window
- **Header Bar**: Contains Controls and About buttons
- **Title Section**: Application title and description
- **Game Cards**: Interactive cards for each game showing:
  - Box art (if available)
  - Game title and year
  - Detailed description
  - Build status (✅ Ready to Play or ❌ Game Not Built)
  - Launch button

### Game Cards Include
- **Manic Miner (1983)**: The original platform adventure game
- **Jet Set Willy (1984)**: The sprawling 60-room sequel

### Features
- **🎮 Controls Button**: Opens detailed controls and features window
- **ℹ️ About Button**: Shows game information and credits
- **🚀 Launch Buttons**: Start games directly from the interface
- **Status Indicators**: Clear visual feedback on game availability

## Error Handling

- **Game Not Found**: Clear error dialog with build instructions
- **Launch Progress**: Shows launching dialog while game starts
- **Completion Status**: Feedback when games finish or encounter errors

## Requirements

- **GTK4**: Modern GUI toolkit
- **Games Built**: Expects games at:
  - `~/ManicMiner/manicminer`
  - `~/JetSetWilly/jetsetwilly`
- **Box Art** (Optional): Images in `~/Downloads/`
- **C++17**: Compatible compiler

## Installation

```bash
# Install to ~/.local/bin/
make install

# Uninstall
make uninstall
```

## Technical Details

- **Framework**: GTK4 with C++17
- **Size**: Lightweight, native Linux application
- **Dependencies**: Only GTK4 and standard library
- **Memory**: Efficient resource usage
- **Threading**: Proper GUI event handling

## Troubleshooting

**Build Errors:**
- Ensure GTK4 is installed: `make check-deps`
- Check compiler: `g++ --version` (needs C++17 support)

**Game Not Found:**
- Build games: `cd ~/ManicMiner && make` or `cd ~/JetSetWilly && make`
- Check executable permissions

**Missing Box Art:**
- Images should be in `~/Downloads/` with exact filenames
- Supported formats: jpg, jpeg, png
- Placeholder will show if images not found

## Cleaning Up

```bash
make clean      # Remove built executable
make uninstall  # Remove from ~/.local/bin/
```

Enjoy the enhanced classic gaming experience! 🎮
