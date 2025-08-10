# Matthew Smith Games - Enhanced Collection

A comprehensive collection of enhanced Matthew Smith ZX Spectrum classics, featuring modern SDL2 ports with beautiful GTK4 launcher.

![Preview](https://img.shields.io/badge/Platform-Linux-blue) ![License](https://img.shields.io/badge/License-Original%20Games%20%C2%A9%20Matthew%20Smith-green) ![Language](https://img.shields.io/badge/Language-C%2FC%2B%2B-orange)

## 🎮 Games Included

### Manic Miner (1983)
The original platform adventure game that defined the genre. Guide Miner Willy through 20 challenging caverns, collecting keys while avoiding deadly robots and hazards.

### Jet Set Willy (1984)  
The sprawling sequel featuring 60 interconnected rooms in Willy's mansion. Explore from the bathroom to the wine cellar, collecting items scattered throughout this ambitious adventure.

## ✨ Enhanced Features

### Modern Enhancements
- **SDL2 Compatibility**: Runs perfectly on modern Linux systems
- **Windowed & Fullscreen**: F11 to toggle, resizable windows
- **Perfect Scaling**: Maintains 4:3 aspect ratio with proper scaling
- **Wayland & X11**: Full compatibility with both display servers
- **Per-pixel Rendering**: Eliminates color clash from original
- **16-Color Palette**: Enhanced color reproduction
- **Polyphonic Audio**: Reproduced music with stereo panning
- **Square Wave Audio**: Authentic "beepy" retro sound

### Bug Fixes
- Fixed original ZX Spectrum bugs (wall collision, etc.)
- 100% identical gameplay with improved reliability
- Enhanced collision detection

## 🎨 Launcher Options

### GUI Launcher (Recommended)
Beautiful GTK4 interface with:
- **Box Art Display**: Shows authentic game covers
- **Game Cards**: Rich information with status indicators  
- **Fullscreen Mode**: Toggle launcher fullscreen
- **Integrated Help**: Built-in controls and about dialogs
- **Native Feel**: Modern Linux-style interface

### Console Launcher
Text-based menu system for those who prefer terminal interfaces.

## 🚀 Quick Start

### Prerequisites
```bash
# CachyOS/Arch Linux
sudo pacman -S sdl2 gtk4 gcc

# Ubuntu/Debian  
sudo apt install libsdl2-dev libgtk-4-dev build-essential

# Fedora
sudo dnf install SDL2-devel gtk4-devel gcc-c++
```

### Build Everything
```bash
# Clone the repository
git clone https://github.com/yourusername/matthew-smith-games-enhanced
cd matthew-smith-games-enhanced

# Build all components
make all-games
make all-launchers
```

### Install Launchers Globally
```bash
make install-launchers
```

### Launch Games
```bash
# Beautiful GUI launcher (recommended)
game-launcher-gui

# Console launcher  
game-launcher

# Direct game execution
cd ManicMiner && ./manicminer
cd JetSetWilly && ./jetsetwilly
```

## 🎮 Game Controls

### Movement
- **Arrow Keys** - Move left/right
- **Space** - Jump
- **Enter** - Confirm/Select
- **Escape** - Return to title screen

### Enhanced Features  
- **F11** - Toggle Fullscreen/Windowed mode
- **TAB/Pause** - Pause/Unpause game
- **M** - Mute/Unmute audio

### Additional (Jet Set Willy)
- **Any Key** - Skip copy protection during loading

### Cheat Mode
- Type the cheat code to activate cheat mode
- Once activated: type level number + Enter to jump to level
- Both Bug-Byte and Software Projects cheat codes supported

## 🏗️ Building Individual Components

### Games
```bash
# Manic Miner
cd ManicMiner
make clean && make

# Jet Set Willy
cd JetSetWilly  
make clean && make
```

### Launchers
```bash
# GUI Launcher
cd GameLauncherGUI
make clean && make && make install

# Console Launcher
cd GameLauncher
make clean && make && make install
```

## 📁 Project Structure

```
matthew-smith-games-enhanced/
├── ManicMiner/              # Enhanced Manic Miner
│   ├── src/                 # Source code
│   ├── linux/              # Build artifacts
│   ├── manicminer           # Game executable
│   ├── Makefile
│   └── CONTROLS.md
├── JetSetWilly/             # Enhanced Jet Set Willy
│   ├── src/                 # Source code  
│   ├── linux/              # Build artifacts
│   ├── jetsetwilly          # Game executable
│   ├── Makefile
│   └── CONTROLS.md
├── GameLauncherGUI/         # GTK4 GUI Launcher
│   ├── main.cpp
│   ├── game-launcher-gui    # GUI executable
│   ├── Makefile
│   └── README.md
├── GameLauncher/            # Console Launcher
│   ├── main.cpp
│   ├── game-launcher        # Console executable
│   ├── Makefile
│   └── README.md
├── Manic_miner_bugbyte.jpg  # Box art
├── Jet_Set_Willy_(game_box_art).jpg
└── README.md                # This file
```

## 🎨 Box Art Integration

The GUI launcher automatically displays game box art:
- Place images in the main project directory
- Supported formats: JPG, PNG, GIF, BMP
- Fallback placeholders if images not found
- Images included in this repository

## 🔧 Technical Details

### Games
- **Language**: C with SDL2
- **Resolution**: 256x192 (ZX Spectrum authentic)  
- **Audio**: 22.05kHz stereo with square wave synthesis
- **Compatibility**: Modern Linux distributions
- **Performance**: Lightweight, minimal resource usage

### GUI Launcher  
- **Framework**: GTK4 with C++17
- **Features**: Modern native Linux interface
- **Dependencies**: GTK4, standard library only
- **Integration**: Desktop file for application menus

### Console Launcher
- **Language**: C++17
- **Interface**: Clean text-based menus  
- **Dependencies**: Standard library only
- **Portability**: Runs everywhere

## 🐛 Troubleshooting

### Build Issues
```bash
# Check dependencies
pkg-config --exists sdl2 && echo "SDL2 OK"
pkg-config --exists gtk4 && echo "GTK4 OK"

# Clean build
make clean-all && make all-games
```

### Runtime Issues  
```bash
# Check executables
ls -la ManicMiner/manicminer JetSetWilly/jetsetwilly

# Test SDL
ldd ManicMiner/manicminer | grep SDL

# Check display  
echo $DISPLAY $WAYLAND_DISPLAY
```

### Game Not Found
- Ensure games are built: `make all-games`
- Check executable permissions
- Verify paths in launcher settings

## 📝 License & Credits

### Original Games
- **Manic Miner** (1983) © Matthew Smith / Bug-Byte / Software Projects
- **Jet Set Willy** (1984) © Matthew Smith / Software Projects

### Enhanced Ports
- C ports with SDL2 enhancements
- Faithful reproduction with modern compatibility
- Bug fixes and improvements while preserving original gameplay

### This Collection
- Enhanced build system and launchers
- Modern Linux integration
- Box art and documentation

## 🤝 Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open Pull Request

### Development Guidelines
- Preserve original gameplay mechanics
- Maintain compatibility with multiple Linux distributions
- Follow existing code style
- Test on both Wayland and X11

## 🎯 Future Enhancements

- [ ] Additional Matthew Smith games
- [ ] Save state functionality  
- [ ] Modern gamepad support
- [ ] Online leaderboards
- [ ] Level editor
- [ ] Custom themes

## 🔗 Links

- [Original Games History](https://en.wikipedia.org/wiki/Matthew_Smith_(games_programmer))
- [ZX Spectrum Information](https://en.wikipedia.org/wiki/ZX_Spectrum)
- [SDL2 Documentation](https://wiki.libsdl.org/SDL2)
- [GTK4 Documentation](https://docs.gtk.org/gtk4/)

---

**Enjoy these classic Matthew Smith masterpieces with modern enhancements!** 🕹️
